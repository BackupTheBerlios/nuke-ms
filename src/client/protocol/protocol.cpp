// protocol.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008, 2009  Alexander Korsunsky
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

#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include<boost/tokenizer.hpp>


#include "protocol/statemachine.hpp"
#include "protocol/protocol.hpp"

#include "bytes.hpp"
#include "msglayer.hpp"


/** @defgroup proto_machine Comunication Protocol State Machine
* @ingroup proto */


using namespace nuke_ms;
using namespace protocol;


static bool parseDestinationString(
    std::string& host,
    std::string& service,
    const std::wstring& where
);


NukeMSProtocol::NukeMSProtocol(const control::notif_callback_t _notification_callback)
            throw()

    : machine_scheduler(true), notification_callback(_notification_callback)
{

    // create an event processor for our state machine

    // Passing _io_service by pointer, because passing references to
    // create_processor does not work.
    event_processor =
        machine_scheduler.create_processor<
            ProtocolMachine,
            control::notif_callback_t
        >(notification_callback);


    // initiate the event processor
    machine_scheduler.initiate_processor(event_processor);

    // machine_thread is in state "not-a-thread", so we will use the move
    // semantics provided by boost::thread to create a new thread and assign it
    // the variable
    machine_thread = boost::thread(
        boost::bind(
            &boost::statechart::fifo_scheduler<>::operator(),
            &machine_scheduler,
            0
            )
        );

}

NukeMSProtocol::~NukeMSProtocol()
{
    // stop the network machine
    machine_scheduler.terminate();

    // catch the running thread
    catchThread(machine_thread, threadwait_ms);
}


void NukeMSProtocol::connect_to(const std::wstring& id)
    throw(std::runtime_error, ProtocolError)
{
    // Get Host/Service pair from the destination string
    std::string host, service;
    if (parseDestinationString(host, service, id)) // on success, pass on event
    {
        // Create new Connection request event
        // and dispatch it to the statemachine
        boost::intrusive_ptr<EvtConnectRequest>
        connect_request(new EvtConnectRequest(host, service));

        machine_scheduler.queue_event(event_processor, connect_request);
    }
    else // on failure, report back to application
    {
        notification_callback(
            control::ReportNotification
                <control::ProtocolNotification::ID_CONNECT_REPORT>(
                    L"Invalid remote site identifier"
            )
        );
    }

}



void NukeMSProtocol::send(const std::wstring& msg)
    throw(std::runtime_error, ProtocolError)
{
    // create a Stringwrapper for the message
    StringwrapLayer::ptr_type data(new StringwrapLayer(msg));

    // Create new Connection request event and dispatch it to the statemachine
    boost::intrusive_ptr<EvtSendMsg>
    send_evt(
        new EvtSendMsg(data)
    );

    machine_scheduler.queue_event(event_processor, send_evt);
}


void NukeMSProtocol::disconnect()
    throw(std::runtime_error, ProtocolError)
{
    // Create new Disconnect request event and dispatch it to the statemachine
    boost::intrusive_ptr<EvtDisconnectRequest>
    disconnect_evt(new EvtDisconnectRequest);

    machine_scheduler.queue_event(event_processor, disconnect_evt);
}


/** Get host and service pair from a single destination string.
* Parses the string containing the destination into a host/service pair.
* The part before the column is the host, the part after the column is the
* service.
* If there is not exactly one column, the function returns false, indicating
* failure.
*
* @param host A reference to a string where the host will be stored.
* Any content will be overwritten.
* @param service A reference to a string where the service will be stored.
* Any content will be overwritten.
* @param where A string containing a destination.
*
* @return true on success, false on failure.
*/
static bool parseDestinationString(
    std::string& host,
    std::string& service,
    const std::wstring& where
)
{
    // get ourself a tokenizer
    typedef boost::tokenizer<boost::char_separator<wchar_t>,
                            std::wstring::const_iterator, std::wstring>
        tokenizer;

    // get the part before the colon and the part after the colon
    boost::char_separator<wchar_t> colons(L":");
    tokenizer tokens(where, colons);

    tokenizer::iterator tok_iter = tokens.begin();

    // bail out, if there were no tokens
    if ( tok_iter == tokens.end() )
        return false;

    // create host from first token
    host.assign(tok_iter->begin(), tok_iter->end());

    if ( ++tok_iter == tokens.end() )
        return false;

    // create service from second token
    service.assign(tok_iter->begin(), tok_iter->end());

    // bail out if there is another colon
    if ( ++tok_iter != tokens.end() )
        return false;

    return true;
}



void nuke_ms::protocol::catchThread(boost::thread& thread, unsigned threadwait_ms)
    throw()
{
    // a thread id that compares equal to "not-a-thread"
    boost::thread::id not_a_thread;

    try {
        // give the thread a few seconds time to join
        thread.timed_join(boost::posix_time::millisec(threadwait_ms));
    }
    catch(...)
    {}

    // if the thread finished, return. otherwise try to kill the thread
    if (thread.get_id() == not_a_thread)
        return;

    thread.interrupt();

    // if it is still running, let it go
    if (thread.get_id() == not_a_thread)
        return;

    thread.detach();
}
