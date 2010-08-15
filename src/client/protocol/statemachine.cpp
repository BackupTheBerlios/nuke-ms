// statemachine.cpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2008, 2009, 2010  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "protocol/statemachine.hpp"

using namespace nuke_ms;
using namespace nuke_ms::protocol;
using namespace boost::asio::ip;

#ifdef I_HATE_THIS_DAMN_BUGGY_STATECHART_LIBRARY
ProtocolMachine::ProtocolMachine(my_context ctx,
    NukeMSProtocol::Signals&  _signals)
    : my_base(ctx), signals(_signals), socket(io_service)
#else
ProtocolMachine::ProtocolMachine(my_context ctx,
    NukeMSProtocol::Signals *_signals)
    : my_base(ctx), signals(*_signals), socket(io_service)
#endif
{}

ProtocolMachine::~ProtocolMachine()
{
    stopIOOperations();
}

void ProtocolMachine::startIOOperations()
    throw(std::exception)
{
    // start a new thread that processes all asynchronous operations
    io_thread = boost::thread(
        boost::bind(
            &boost::asio::io_service::run,
            &outermost_context().io_service
        )
    );
}

void ProtocolMachine::stopIOOperations() throw()
{
    boost::system::error_code dontcare;

    // cancel all operations and close the socket
    socket.close(dontcare);

    // stop the service object if it's running
    io_service.stop();

    // stop the thread
    catchThread(io_thread, thread_timeout);

    // reset the io_service object so it can continue its work afterwards
    io_service.reset();
}


StateWaiting::StateWaiting(my_context ctx)
    : my_base(ctx)
{
    std::cout<<"Entering StateWaiting\n";

    // when we are waiting, we don't need the io_service object and the thread
    outermost_context().stopIOOperations();
}



boost::statechart::result StateWaiting::react(const EvtConnectRequest& evt)
{
     //get a resolver
    boost::shared_ptr<tcp::resolver> resolver(
        new tcp::resolver(outermost_context().io_service)
    );

    // create a query
    boost::shared_ptr<tcp::resolver::query> query(
        new tcp::resolver::query(evt.host, evt.service)
    );


    // dispatch an asynchronous resolve request
    resolver->async_resolve(
        *query,
        boost::bind(
            &StateNegotiating::resolveHandler,
            boost::asio::placeholders::error,
            boost::asio::placeholders::iterator,
            boost::ref(outermost_context()),
            resolver, query
        )
    );

#if 0
    boost::shared_ptr<boost::asio::deadline_timer> timer(
        new boost::asio::deadline_timer(
            outermost_context().io_service, boost::posix_time::seconds(5)
        )
    );

    timer->async_wait(
        boost::bind(
            StateNegotiating::tiktakHandler,
            boost::asio::placeholders::error,
            timer,
            boost::ref(outermost_context())
        )
    );
#endif

    return transit< StateNegotiating >();
}

boost::statechart::result StateWaiting::react(const EvtSendMsg& evt)
{
    control::SendReport::ptr_t rprt(new control::SendReport);
    rprt->send_state = false;
    rprt->reason = control::SendReport::SR_SERVER_NOT_CONNECTED;
    rprt->reason_str = L"Not Connected.";

    context<ProtocolMachine>().signals.sendReport(rprt);

    return discard_event();
}



StateNegotiating::StateNegotiating(my_context ctx)
    : my_base(ctx)
{
    std::cout<<"Entering StateNegotiating\n";

    try {
        outermost_context().startIOOperations();
    }
    catch(const std::exception& e)
    {
        std::string errmsg(e.what());
        errmsg = "Internal error:" + errmsg;

        post_event(
            EvtConnectReport(
                false,
                byte_traits::string(errmsg.begin(), errmsg.end())
            )
        );
    }
    catch(...)
    {
        post_event(
            EvtConnectReport(
                false,
                byte_traits::string(L"Unknown internal Error")
            )
        );
    }

}

boost::statechart::result StateNegotiating::react(const EvtSendMsg& evt)
{
    control::SendReport::ptr_t rprt(new control::SendReport);
    rprt->send_state = false;
    rprt->reason = control::SendReport::SR_SERVER_NOT_CONNECTED;
    rprt->reason_str = L"Not yet Connected.";

    context<ProtocolMachine>().signals.sendReport(rprt);

    return discard_event();
}

boost::statechart::result StateNegotiating::react(const EvtConnectReport& evt)
{
    control::ConnectionStatusReport::ptr_t
        rprt(new control::ConnectionStatusReport);

    // change state according to the outcome of a connection attempt
    if ( evt.success )
    {
        rprt->newstate = control::ConnectionStatusReport::CNST_CONNECTED;
        rprt->statechange_reason =
            control::ConnectionStatusReport::STCHR_USER_REQUESTED;
        rprt->msg = byte_traits::string(evt.message);
        context<ProtocolMachine>().signals.connectStatReport(rprt);

        return transit<StateConnected>();
    }
    else
    {
        rprt->newstate = control::ConnectionStatusReport::CNST_DISCONNECTED;
        rprt->statechange_reason =
            control::ConnectionStatusReport::STCHR_CONNECT_FAILED;
        rprt->msg = byte_traits::string(evt.message);
        context<ProtocolMachine>().signals.connectStatReport(rprt);

        return transit<StateWaiting>();
    }
}

boost::statechart::result StateNegotiating::react(const EvtDisconnectRequest&)
{
    control::ConnectionStatusReport::ptr_t
    rprt(new control::ConnectionStatusReport);

    rprt->newstate = control::ConnectionStatusReport::CNST_DISCONNECTED;
    rprt->statechange_reason =
        control::ConnectionStatusReport::STCHR_USER_REQUESTED;
    context<ProtocolMachine>().signals.connectStatReport(rprt);

    return transit<StateWaiting>();
}

boost::statechart::result StateNegotiating::react(const EvtConnectRequest&)
{
    control::ConnectionStatusReport::ptr_t
        rprt(new control::ConnectionStatusReport);

    rprt->newstate = control::ConnectionStatusReport::CNST_CONNECTING;
    rprt->statechange_reason =
        control::ConnectionStatusReport::STCHR_BUSY;
    rprt->msg = L"Currently trying to connect";
    context<ProtocolMachine>().signals.connectStatReport(rprt);

    return discard_event();
}


#if 0
void StateNegotiating::tiktakHandler(
    const boost::system::error_code& error,
    boost::shared_ptr<boost::asio::deadline_timer> timer,
    outermost_context_type& _outermost_context
)
{
    std::cout<<"Timer went off: "<<error.message()<<'\n';
}
#endif

void StateNegotiating::resolveHandler(
    const boost::system::error_code& error,
    tcp::resolver::iterator endpoint_iterator,
    outermost_context_type& _outermost_context,
    boost::shared_ptr<tcp::resolver> /* resolver */,
    boost::shared_ptr<tcp::resolver::query> /* query */
)
{
    std::cout<<"resolveHandler invoked.\n";

    // if there was an error, report it
    if (endpoint_iterator == tcp::resolver::iterator() )
    {
        std::string errmsg;

        if (error)
            errmsg = error.message();
        else
            errmsg = "No hosts found.";

        _outermost_context.my_scheduler().queue_event(
            _outermost_context.my_handle(),
            boost::intrusive_ptr<EvtConnectReport> (
                new EvtConnectReport(
                    false,
                    byte_traits::string(errmsg.begin(),errmsg.end())
                )
            )
        );

        return;
    }

    std::cout<<"Resolving finished. Host: "<<
        endpoint_iterator->endpoint().address().to_string()<<" Port: "<<
        endpoint_iterator->endpoint().port()<<'\n';

    _outermost_context.socket.async_connect(
        *endpoint_iterator,
        boost::bind(
            &StateNegotiating::connectHandler,
            boost::asio::placeholders::error,
            boost::ref(_outermost_context)
        )
    );
}




void StateNegotiating::connectHandler(
    const boost::system::error_code& error,
    outermost_context_type& _outermost_context
)
{
    std::cout<<"connectHandler invoked.\n";

    boost::intrusive_ptr<EvtConnectReport> evt_rprt;

    // if there was an error, create a negative reply
    if (error)
    {
        std::string errmsg(error.message());

        evt_rprt = new EvtConnectReport(
            false,
            byte_traits::string(errmsg.begin(), errmsg.end())
        );
    }
    else // if there was no error, create a positive reply
    {
        evt_rprt = new EvtConnectReport(
            true,
            byte_traits::string(L"Connection succeeded.")
        );

        // create a receive buffer
        byte_traits::byte_t* rcvbuf =
            new byte_traits::byte_t[SegmentationLayer::header_length];

        // start an asynchronous read to receive the header of the first packet
        async_read(
            _outermost_context.socket,
            boost::asio::buffer(rcvbuf, SegmentationLayer::header_length),
            boost::bind(
                &StateConnected::receiveSegmentationHeaderHandler,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred,
                boost::ref(_outermost_context),
                rcvbuf
            )
        );
    }

    _outermost_context.my_scheduler().
        queue_event(_outermost_context.my_handle(), evt_rprt);
}





StateConnected::StateConnected(my_context ctx)
    : my_base(ctx)
{
    std::cout<<"Entering StateConnected\n";
}


boost::statechart::result StateConnected::react(const EvtDisconnectRequest&)
{
    control::ConnectionStatusReport::ptr_t
    rprt(new control::ConnectionStatusReport);

    rprt->newstate = control::ConnectionStatusReport::CNST_DISCONNECTED;
    rprt->statechange_reason =
        control::ConnectionStatusReport::STCHR_USER_REQUESTED;
    context<ProtocolMachine>().signals.connectStatReport(rprt);

    return transit<StateWaiting>();
}


boost::statechart::result StateConnected::react(const EvtSendMsg& evt)
{
    // create segmentation layer from the data to be sent
    SegmentationLayer segm_layer(evt.data);

    // create buffer, fill it with the serialized message
    SegmentationLayer::dataptr_t data(
        new byte_traits::byte_sequence(segm_layer.getSerializedSize())
    );

    segm_layer.fillSerialized(data->begin());

    async_write(
        context<ProtocolMachine>().socket,
        boost::asio::buffer(*data),
        boost::bind(
            &StateConnected::writeHandler,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            boost::ref(outermost_context()),
            data
        )
    );

    return discard_event();
}


boost::statechart::result StateConnected::react(const EvtDisconnected& evt)
{
    control::ConnectionStatusReport::ptr_t
        rprt(new control::ConnectionStatusReport);

    rprt->newstate = control::ConnectionStatusReport::CNST_DISCONNECTED;
    rprt->statechange_reason =
        control::ConnectionStatusReport::STCHR_SOCKET_CLOSED;
    rprt->msg = evt.msg;
    context<ProtocolMachine>().signals.connectStatReport(rprt);

    return transit<StateWaiting>();
}


boost::statechart::result StateConnected::react(const EvtRcvdMessage& evt)
{
    // FIXME data implicitly treated as a string, which might not be
    // correct

    // create string from received data
    StringwrapLayer str(evt.data.getUpperLayer()->getSerializedData());

    control::Message::ptr_t msg(new control::Message);
    msg->str = str.getString();

    context<ProtocolMachine>().signals.rcvMessage(msg);

    return discard_event();
}


boost::statechart::result StateConnected::react(const EvtConnectRequest&)
{
    control::ConnectionStatusReport::ptr_t
        rprt(new control::ConnectionStatusReport);

    rprt->newstate = control::ConnectionStatusReport::CNST_CONNECTED;
    rprt->statechange_reason =
        control::ConnectionStatusReport::STCHR_BUSY;
    rprt->msg = L"Allready connected";
    context<ProtocolMachine>().signals.connectStatReport(rprt);

    return discard_event();
}



void StateConnected::writeHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    outermost_context_type& _outermost_context,
    SegmentationLayer::dataptr_t data
)
{
    std::cout<<"Sending message finished\n";


    if (!error)
    {
        control::SendReport::ptr_t rprt(new control::SendReport);
        rprt->send_state = true;
        rprt->reason = control::SendReport::SR_SEND_OK;

        _outermost_context.signals.sendReport(rprt);
    }
    else
    {
        std::string errmsg(error.message());

        control::SendReport::ptr_t rprt(new control::SendReport);
        rprt->send_state = false;
        rprt->reason = control::SendReport::SR_CONNECTION_ERROR;
        rprt->reason_str = byte_traits::string(errmsg.begin(), errmsg.end());

        _outermost_context.signals.sendReport(rprt);

        _outermost_context.my_scheduler().queue_event(
            _outermost_context.my_handle(),
            boost::intrusive_ptr<EvtDisconnected> (
                new EvtDisconnected(
                    byte_traits::string(errmsg.begin(), errmsg.end())
                )
            )
        );

    }
}

void StateConnected::receiveSegmentationHeaderHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    outermost_context_type& _outermost_context,
    byte_traits::byte_t rcvbuf[SegmentationLayer::header_length]
)
{
    std::cout<<"Reveived message\n";

    // if there was an error,
    // tear down the connection by posting a disconnection event
    if (error || bytes_transferred != SegmentationLayer::header_length)
    {
        std::string errmsg(error.message());

        _outermost_context.my_scheduler().queue_event(
            _outermost_context.my_handle(),
            boost::intrusive_ptr<EvtDisconnected>(
                new EvtDisconnected(byte_traits::string(errmsg.begin(),errmsg.end()))
            )
        );
    }
    else // if no error occured, try to decode the header
    {
        try {
            // decode and verify the header of the message
            SegmentationLayer::HeaderType header_data
                = SegmentationLayer::decodeHeader(rcvbuf);

/// @fixme Magic number, set to something proper or make configurable
const byte_traits::uint2b_t MAX_PACKETSIZE = 0x8FFF;

            if (header_data.packetsize > MAX_PACKETSIZE)
                throw MsgLayerError("Oversized packet.");


            SegmentationLayer::dataptr_t body_buf(
                new byte_traits::byte_sequence(
                    header_data.packetsize-SegmentationLayer::header_length
                )
            );

            // start an asynchronous receive for the body
            async_read(
                _outermost_context.socket,
                boost::asio::buffer(*body_buf),
                boost::bind(
                    &StateConnected::receiveSegmentationBodyHandler,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred,
                    boost::ref(_outermost_context),
                    body_buf
                )
            );
        }
        // on failure, report back to application
        catch (const std::exception& e)
        {
            std::string errmsg(e.what());

            _outermost_context.my_scheduler().queue_event(
                _outermost_context.my_handle(),
                boost::intrusive_ptr<EvtDisconnected>(
                    new EvtDisconnected(
                        byte_traits::string(errmsg.begin(),errmsg.end())
                    )
                )
            );
        }
        catch(...)
        {
            _outermost_context.my_scheduler().queue_event(
                _outermost_context.my_handle(),
                boost::intrusive_ptr<EvtDisconnected>(
                    new EvtDisconnected(L"Unknown Error")
                )
            );
        }
    }

    delete[] rcvbuf;
}

void StateConnected::receiveSegmentationBodyHandler(
    const boost::system::error_code& error,
    std::size_t bytes_transferred,
    outermost_context_type& _outermost_context,
    SegmentationLayer::dataptr_t rcvbuf
)
{
    // if there was an error,
    // tear down the connection by posting a disconnection event
    if (error)
    {
        std::string errmsg(error.message());

        _outermost_context.my_scheduler().queue_event(
            _outermost_context.my_handle(),
            boost::intrusive_ptr<EvtDisconnected>(
                new EvtDisconnected(byte_traits::string(errmsg.begin(),errmsg.end()))
            )
        );
    }
    else // if no error occured, report the received message to the application
    {
        SegmentationLayer segmlayer(rcvbuf);

        _outermost_context.my_scheduler().queue_event(
            _outermost_context.my_handle(),
            boost::intrusive_ptr<EvtRcvdMessage>(
                new EvtRcvdMessage(segmlayer)
            )
        );

        // start a new receive for the next message

        // create a receive buffer
        byte_traits::byte_t* rcvbuf =
            new byte_traits::byte_t[SegmentationLayer::header_length];

        // start an asynchronous read to receive the header of the first packet
        async_read(
            _outermost_context.socket,
            boost::asio::buffer(rcvbuf, SegmentationLayer::header_length),
            boost::bind(
                &StateConnected::receiveSegmentationHeaderHandler,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred,
                boost::ref(_outermost_context),
                rcvbuf
            )
        );
    }
}

