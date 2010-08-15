// sigtypes.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2010  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef SIGTYPES_HPP
#define SIGTYPES_HPP

#include <boost/shared_ptr.hpp>
#include "bytes.hpp"

namespace nuke_ms
{
namespace control
{


/** Identification of the server location */
struct ServerLocation
{
    typedef boost::shared_ptr<ServerLocation> ptr_t;
    typedef boost::shared_ptr<const ServerLocation> const_ptr_t;

    byte_traits::string where; /**< String with hostname or ip address */
};

/** Message that is sent or received over the network.
*/
struct Message
{
    typedef boost::shared_ptr<Message> ptr_t;
    typedef boost::shared_ptr<const Message> const_ptr_t;

    /** Type for unique message ID */
    typedef unsigned short message_id_t;

    message_id_t id; /**< Unique message ID */
    byte_traits::string str; /**< Message string */
};

/** Status report of connection state changes
*/
struct ConnectionStatusReport
{
    typedef boost::shared_ptr<ConnectionStatusReport> ptr_t;
    typedef boost::shared_ptr<const ConnectionStatusReport> const_ptr_t;

    /** Type for the current connection state */
    enum connect_state_t
    {
        CNST_DISCONNECTED,
        CNST_CONNECTING,
        CNST_CONNECTED
    };

    /** Type for the reason of a state change */
    enum statechange_reason_t
    {
        STCHR_NO_REASON = 0, /**<No reason, used for example after state query*/
        STCHR_INTERNAL_ERROR, /**< Internal error has occured, refer to msg */
        STCHR_CONNECT_SUCCESSFUL, /**< Connection attempt was successful */
        STCHR_CONNECT_FAILED, /**< Connection attempt failed */
        STCHR_SOCKET_CLOSED, /**< Connection to remote server lost */
        STCHR_USER_REQUESTED, /**< User requested state change */
        STCHR_BUSY /**< An operation is currently being performed */
    };

    connect_state_t newstate; /**< current connection state */
    statechange_reason_t statechange_reason; /**< reason for state change */
    byte_traits::string msg; /**< Optional message describing the reason */
};


/** Report for sent messages
*/
struct SendReport
{
    typedef boost::shared_ptr<SendReport> ptr_t;
    typedef boost::shared_ptr<const SendReport> const_ptr_t;


    Message::message_id_t message_id; /**< ID of the message in question */
    bool send_state; /** Was it sent or not */

    enum send_rprt_reason_t
    {
        SR_SEND_OK, /**< All cool. */
        SR_SERVER_NOT_CONNECTED, /**< Not connected to server */
        SR_CONNECTION_ERROR /**< Network failure */
    } reason;
    byte_traits::string reason_str;
};


// Signals issued by the GUI
typedef boost::signals2::signal<void (control::ServerLocation::const_ptr_t)>
    SignalConnectTo;
typedef boost::signals2::signal<void (control::Message::const_ptr_t)>
    SignalSendMessage;
typedef boost::signals2::signal<void ()> SignalConnectionStatusQuery;
typedef boost::signals2::signal<void ()> SignalDisconnect;



// Signals issued by the Protocol
typedef boost::signals2::signal<void (control::Message::const_ptr_t)>
    SignalRcvMessage;
typedef boost::signals2::signal<
    void (control::ConnectionStatusReport::const_ptr_t)>
    SignalConnectionStatusReport;
typedef boost::signals2::signal<
    void (control::SendReport::const_ptr_t)>
    SignalSendReport;

} // namespace control
} // namespace nuke_ms


#endif // ifndef SIGTYPES_HPP

