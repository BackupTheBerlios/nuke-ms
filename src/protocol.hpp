// protocol.hpp

/*
 *   NMS - Nuclear Messaging System
 *   Copyright (C) 2008  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/** @file protocol.hpp
* Network communication protocol
*/

/** @defgroup proto Communication Protocol */
/** @defgroup proto_machine Comunication Protocol State Machine
* @ingroup proto */


#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <stdexcept>

#include <boost/asio.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>


#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include <boost/function.hpp>

#include "control.hpp"

namespace nms
{
namespace protocol
{

using boost::asio::ip::tcp;


/** Class for errors that can be issued by the Communication Protocol.
*/
class ProtocolError : public std::runtime_error
{
    /** The error message */
    const char* msg;
public:
    /** Default Constructor */
    ProtocolError() throw()
        : std::runtime_error("Unknown Communication Protocol Error")
    { }

    /** Constructor. 
    * @param str The error message
    */
    ProtocolError(const char* str) throw()
        : std::runtime_error(str)
    {}

    /** Return error message as char array.
    * @return A null- terminated character array containg the error message
    */
    virtual const char* what() const throw()
    { return std::runtime_error::what(); }

    virtual ~ProtocolError() throw()
    { }
};






struct EventConnectRequest : boost::statechart::event<EventConnectRequest>
{
    std::wstring where;
};

struct EventConnectReport : boost::statechart::event<EventConnectRequest>
{
    bool success;
    std::wstring reason;
};

struct EventRcvdMsg : boost::statechart::event<EventRcvdMsg>
{
    std::wstring msg;
};


struct EventSendMsg : boost::statechart::event<EventSendMsg>
{
    std::wstring msg;
};

struct EventDisconnect : boost::statechart::event<EventDisconnect>
{};




struct StateUnconnected;
struct StateConnected;


struct ProtocolMachine
    : public boost::statechart::state_machine<ProtocolMachine, StateUnconnected>
{
    boost::function1 <void, const control::ProtocolNotification&>
        notification_callback;

    ProtocolMachine(const boost::function1
                        <void, const control::ProtocolNotification&>&
                        _notification_callback) throw()
        : notification_callback(_notification_callback)
    {}
};





struct StateIdle;
struct StateTryingConnect;

struct StateUnconnected
    : public boost::statechart::simple_state<StateUnconnected,
                                                ProtocolMachine,
                                                StateIdle>
{ };


struct StateIdle
    : public boost::statechart::simple_state<StateIdle,
                                                StateUnconnected>
{
    typedef boost::statechart::custom_reaction< EventConnectRequest > reactions;

    boost::statechart::result react(const EventConnectRequest& request)
    {
        std::cout<<"Trying to connect to "<<std::string(request.where.begin(), request.where.end())<<"\n";
        return transit<StateTryingConnect>();
    }
};

struct StateTryingConnect
    : public boost::statechart::simple_state<StateTryingConnect,
                                                StateUnconnected>
{
    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EventConnectReport >,
        boost::statechart::custom_reaction<EventDisconnect>
    > reactions;

    boost::statechart::result react(const EventConnectReport& rprt)
    {
        if (rprt.success)
        {
            context<ProtocolMachine>()
                .notification_callback(
                    control::ReportNotification
                        <control::ProtocolNotification::ID_CONNECT_REPORT>()
                    );

            return transit<StateConnected>();
        }
        else
        {
            context<ProtocolMachine>()
                .notification_callback(
                    control::ReportNotification
                        <control::ProtocolNotification::ID_CONNECT_REPORT>
                        (L"Sorry, connection failed.\n")
                    );

            return transit<StateUnconnected>();
        }
    }

    boost::statechart::result react(const EventDisconnect&)
    {
        context<ProtocolMachine>()
            .notification_callback(
                control::ProtocolNotification
                    (control::ProtocolNotification::ID_DISCONNECTED)
                );

        return transit<StateUnconnected>();
    }

};


struct StateConnected
    : public boost::statechart::simple_state<StateConnected,
                                                ProtocolMachine>
{
    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EventSendMsg >,
        boost::statechart::custom_reaction< EventRcvdMsg >,
        boost::statechart::custom_reaction<EventDisconnect>
    > reactions;

    StateConnected()
    {
        std::cout<<"We are connected!\n";
    }

    
    boost::statechart::result react(const EventSendMsg& msg)
    {
        std::cout<<"Sending message: "<<std::string(msg.msg.begin(), msg.msg.end())<<'\n';
        return discard_event();
    }

    boost::statechart::result react(const EventRcvdMsg& msg)
    {
        context<ProtocolMachine>()
            .notification_callback(
                control::ReceivedMsgNotification
                    (msg.msg)
                );

        return discard_event();
    }

    boost::statechart::result react(const EventDisconnect&)
    {
        context<ProtocolMachine>()
            .notification_callback(
                control::ProtocolNotification
                    (control::ProtocolNotification::ID_DISCONNECTED)
                );

        return transit<StateUnconnected>();
    }

};



/** Client Communication Protocol.
* @ingroup proto
* Use this class to create a black box that handles all the network stuff for 
* you.
* Request are handled by the public functions connect_to, send and disconnect.
* Replies will be dispatched to the callback function you supply in the constructor.
*/
class NMSProtocol
{
    boost::mutex machine_mutex; /**< the mutex for the state machine*/
    ProtocolMachine protocol_machine; /**< The state machine itself*/
    
    /** The function object that will be called, if an event occurs.*/
    boost::function1<void, const control::ProtocolNotification&>
        notification_callback;

public:

    /** Constructor.
    * Initializes the Network machine.
    * @param _notification_callback The callback function where the events will 
    * be dispatched
    */
    NMSProtocol(const boost::function1<void,
                                        const control::ProtocolNotification&>&
                    _notification_callback) throw()
        : notification_callback(_notification_callback),
            protocol_machine(_notification_callback)
    {
        protocol_machine.initiate();
    }

    /** Connect to a remote site.
     * @param id The string representation of the address of the remote site
    * @throws std::runtime_error if a ressource could not be allocated. 
    * e.g. a threading resource.
    * @throws ProtocolError if a networking error occured
     */
    void connect_to(const std::wstring& id)
        throw(std::runtime_error, ProtocolError)
    {
        boost::lock_guard<boost::mutex> guard(machine_mutex);

        protocol_machine.process_event(EventConnectRequest(id));
        protocol_machine.process_event(EventConnectReport(true));
    }

    /** Send message to connected remote site.
     * @param msg The message you want to send
    * @throws std::runtime_error if a ressource could not be allocated. 
    * e.g. a threading resource.
    * @throws ProtocolError if a networking error occured
     */
    void send(const std::wstring& msg)
        throw(std::runtime_error, ProtocolError)
    {
        boost::lock_guard<boost::mutex> guard(machine_mutex);
        protocol_machine.process_event(EventSendMsg(msg));
    }

    /** Disconnect from the remote site.
    * @throws std::runtime_error if a ressource could not be allocated. 
    * e.g. a threading resource.
    * @throws ProtocolError if a networking error occured
    */
    void disconnect()
        throw(std::runtime_error, ProtocolError)
    {
        boost::lock_guard<boost::mutex> guard(machine_mutex);
        protocol_machine.process_event(EventDisconnect());
    }

};

} // namespace protocol
} // namespace nms

#endif /*PROTOCOL_HPP_*/
