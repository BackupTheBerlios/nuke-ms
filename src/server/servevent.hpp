// servevent.hpp

/*
 *   NMS - Nuclear Messaging System
 *   Copyright (C) 2009  Alexander Korsunsky
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

#ifndef SERVEVENT_HPP
#define SERVEVENT_HPP

#include <boost/function.hpp>

namespace nms
{

/** Basic event sent to the server.
* This event will be passed to an event handler function when something
* interesting happens to the Socket, such as a received message or a
* disconnection. The handler function then takes the appropriate actions.
* To identify what kind of event has happened, an enum variable has to be set
* by the constructor.
* To identify the sender of the event, an id variable has to be set by the
* constructor.
*/
struct BasicServerEvent
{
    /** Type that identifies the connection to which this event happened. */
    typedef int connection_id_t;

    /** Enum for the kind of the Event that happened. */
    enum event_kind_t {
        ID_MSG_RECEIVED,
        ID_CAN_DELETE
    };

    event_kind_t event_kind; /**< What kind of event has happened */
    connection_id_t connection_id; /**< To whom the event happened */

    /** Constructor.
    * Sets the event kind and connection id variables.
    */
    BasicServerEvent(
        event_kind_t _event_kind,
        connection_id_t _connection_id
    )
        : event_kind(_event_kind), connection_id(_connection_id)
    {}

    virtual ~BasicServerEvent() {}
};

/**  Server Event with a message.
* This class template is to be used for Server events, which contain a single
* message string.
*/
template <BasicServerEvent::event_kind_t EventKind>
struct ServerEventMessage : public BasicServerEvent
{
    /** The message that was received */
    const std::wstring msg;

    /** Constructor.
    * @param _msg The message that was received
    */
    ServerEventMessage(
        BasicServerEvent::connection_id_t _connection_id,
        const std::wstring& _msg
    )  throw()
        :  BasicServerEvent(EventKind, _connection_id), msg(_msg)
    {}

    virtual ~ServerEventMessage() {}
};

/** Typedef for received message. */
typedef ServerEventMessage<BasicServerEvent::ID_MSG_RECEIVED>
    ReceivedMessageEvent;

/** Typedef for Disconnection events. */
typedef ServerEventMessage<BasicServerEvent::ID_CAN_DELETE>
    DisconnectedEvent;



/** A typedef for the notification callback */
typedef boost::function1 <void, const BasicServerEvent&>
    event_callback_t;

} // nms

#endif // ifndef SERVEVENT_HPP