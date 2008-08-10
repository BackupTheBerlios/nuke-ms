// notifications.hpp

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

/** @todo Add file documentation */

/** @defgroup  notif Notification Messages
* @ingroup app_ctrl */


#ifndef NOTIFICATIONS_HPP
#define NOTIFICATIONS_HPP


namespace nms
{
namespace control
{



/** A notification from the Communication protocol regarding an event
* @ingroup notif
*/
struct ProtocolNotification
{
    /** The type of operation that is reported about.
    */
    enum notification_id_t {
        ID_CONNECT_REPORT, /**< Report regarding a connection attempt */
        ID_SEND_REPORT, /**< Report regarding a sent message */
        ID_RECEIVED_MSG, /**< Message received */
        ID_DISCONNECTED /**< Disconnected */
    };

    /** What kind of Notification */
    const notification_id_t id;

    /** Construct a Report denoting failure */
    ProtocolNotification(notification_id_t _id)  throw()
        : id(_id)
    {}

    virtual ~ProtocolNotification()
    {}
};

/** Notification regarding a received message
* @ingroup notif*/
struct ReceivedMsgNotification : public ProtocolNotification
{
    /** The message that was received */
    const std::wstring msg;

    /** Constructor.
    * @param _msg The message that was received
    */
    ReceivedMsgNotification(const std::wstring& _msg)  throw()
        : msg(_msg), ProtocolNotification(ID_RECEIVED_MSG)
    {}
};


/** This class represents a positive or negative reply to a requested operation.
* @ingroup notif
*/
struct RequestReport
{
    const bool successful; /**< Good news or bad news? */
    const std::wstring failure_reason; /**< What went wrong? */

    /** Construct a positive report */
    RequestReport()  throw()
        : successful(true)
    {}

    /** Construct a negative report */
    RequestReport(const std::wstring& reason)  throw()
        : successful(false), failure_reason(reason)
    {}

    virtual ~RequestReport()
    {}
};

/** A generic notification that involves reporting about Success or failure */
template <ProtocolNotification::notification_id_t NOTIF_ID>
struct ReportNotification : public ProtocolNotification, public RequestReport
{
    /** Generate a successful report */
    ReportNotification()  throw()
        : ProtocolNotification(NOTIF_ID), RequestReport()
    {}

    /** Generate a negative report */
    ReportNotification(const std::wstring& reason)  throw()
        : ProtocolNotification(NOTIF_ID), RequestReport(reason)
    {}
};


} // namespace control

} // namespace nms

#endif // ifndef NOTIFICATIONS_HPP
