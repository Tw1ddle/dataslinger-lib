#include "dataslinger/websocket/datareceiverwebsocket.h"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "dataslinger/connection/connectioninfo.h"
#include "dataslinger/event/event.h"

namespace
{

// Sends a WebSocket message and prints the response
class DataReceiverWebSocketSession : public std::enable_shared_from_this<DataReceiverWebSocketSession>
{
public:
    DataReceiverWebSocketSession(const dataslinger::connection::ConnectionInfo& info) : m_resolver(m_ioContext), m_socketStream(m_ioContext)
    {
        if(!info.hasWebSocketReceiverInfo()) {
            queueFatalEvent("Will fail to create receiver, missing connection info");
            return;
        }

        // Save these for later
        const std::uint16_t port = info.getInfo().getValue<std::uint16_t>(dataslinger::connection::ConnectionInfoDataKeys::WEBSOCKET_RECEIVER_PORT_UINT16);
        m_host = info.getInfo().getValue<std::string>(dataslinger::connection::ConnectionInfoDataKeys::WEBSOCKET_RECEIVER_HOST_STRING);
        m_port = std::to_string(port);
    }

    void run()
    {
        // Start the asynchronous operation - look up the domain name
        m_resolver.async_resolve(m_host, m_port,
            std::bind(&DataReceiverWebSocketSession::onResolve, shared_from_this(), std::placeholders::_1, std::placeholders::_2));

        m_ioContext.run();
    }

    void poll(const std::function<void(const dataslinger::message::Message&)>& onReceive,
              const std::function<void(const dataslinger::event::Event&)>& onEvent)
    {
        m_messageQueue.consume_all([&onReceive](const dataslinger::message::Message m) {
            onReceive(m);
        });
        m_eventQueue.consume_all([&onEvent](const dataslinger::event::Event e) {
            onEvent(e);
        });
    }

    void stop()
    {
        m_ioContext.stop();
    }

private:
    void onResolve(const boost::system::error_code ec, const boost::asio::ip::tcp::resolver::results_type results)
    {
        if(ec) {
            queueFatalEvent(ec, "resolve");
            return;
        }

        queueInformationalEvent("Will make the connection on the IP address we resolved from lookup");

        boost::asio::async_connect(m_socketStream.next_layer(), results.begin(), results.end(),
            std::bind(&DataReceiverWebSocketSession::onConnect, shared_from_this(), std::placeholders::_1));
    }

    void onConnect(const boost::system::error_code ec)
    {
        if(ec) {
            queueFatalEvent(ec, "connect");
            return;
        }

        queueInformationalEvent("Did connect, will asynchronously send the websocket upgrade request handshake");

        m_socketStream.async_handshake(m_host, "/", std::bind(&DataReceiverWebSocketSession::onHandshake, shared_from_this(), std::placeholders::_1));
    }

    void onHandshake(const boost::system::error_code ec)
    {
        if(ec) {
            queueFatalEvent(ec, "handshake");
            return;
        }

        queueInformationalEvent("Did receive websocket upgrade handshake response");

        // Send the message
        m_socketStream.async_write(boost::asio::buffer("TODO - text message"),
            std::bind(&DataReceiverWebSocketSession::onWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void onWrite(const boost::system::error_code ec, const std::size_t bytesTransferred)
    {
        boost::ignore_unused(bytesTransferred);

        if(ec) {
            queueFatalEvent(ec, "write");
            return;
        }

        queueInformationalEvent("receiver onwrite");

        // Read a message into our buffer
        m_socketStream.async_read(m_buffer,
            std::bind(&DataReceiverWebSocketSession::onRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void onRead(const boost::system::error_code ec, const std::size_t bytesTransferred)
    {
        boost::ignore_unused(bytesTransferred);

        if(ec) {
            queueFatalEvent(ec, "read");
            return;
        }

        queueInformationalEvent("read message into buffer");

        // TODO only close when we ask it to
        // Close the WebSocket connection
        //m_socketStream.async_close(boost::beast::websocket::close_code::normal,
        //    std::bind(&DataReceiverWebSocketSession::onClose, shared_from_this(), std::placeholders::_1));
    }

    void onClose(const boost::system::error_code ec)
    {
        if(ec) {
            queueFatalEvent(ec, "close");
            return;
        }

        queueInformationalEvent("closing receiver connection gracefully");
        // If we get here then the connection is closed gracefully
    }

    // Report a fatal error
    void queueFatalEvent(const boost::system::error_code ec, const std::string what)
    {
        const std::string msg = std::string(what).append("_").append(ec.message());

        m_eventQueue.push(dataslinger::event::Event({{{
            { dataslinger::event::EventDataKeys::INFORMATIONAL_MESSAGE_STRING, msg }
        }}}));
    }
    void queueFatalEvent(const std::string what)
    {
        m_eventQueue.push(dataslinger::event::Event({{{
            { dataslinger::event::EventDataKeys::INFORMATIONAL_MESSAGE_STRING, what }
        }}}));
    }

    // Report an informational event
    void queueInformationalEvent(const std::string what)
    {
        m_eventQueue.push(dataslinger::event::Event({{{
            { dataslinger::event::EventDataKeys::INFORMATIONAL_MESSAGE_STRING, what }
        }}}));
    }

    boost::asio::io_context m_ioContext;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_socketStream;
    boost::beast::multi_buffer m_buffer;
    std::string m_host;
    std::string m_port;

    // TODO ideally bound capacity by memory used or make unlimited
    boost::lockfree::spsc_queue<dataslinger::message::Message, boost::lockfree::fixed_sized<true>> m_messageQueue{10000}; ///< Queue that grows as receiver receives messages
    boost::lockfree::spsc_queue<dataslinger::event::Event, boost::lockfree::fixed_sized<true>> m_eventQueue{10000}; ///< Queue of internal events generated by the receiver
};

}

namespace dataslinger
{
namespace websocket
{

class DataReceiverWebSocket::DataReceiverWebSocketImpl
{
public:
    DataReceiverWebSocketImpl(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionInfo& info)
        : m_onReceive{onReceive}, m_onEvent{onEvent}
    {
        m_session = std::make_shared<DataReceiverWebSocketSession>(info);
    }

    ~DataReceiverWebSocketImpl()
    {
        m_session->stop();
    }
    DataReceiverWebSocketImpl(const DataReceiverWebSocketImpl&) = delete;
    DataReceiverWebSocketImpl& operator=(const DataReceiverWebSocketImpl&) = delete;
    DataReceiverWebSocketImpl(DataReceiverWebSocketImpl&&) = default;
    DataReceiverWebSocketImpl& operator=(DataReceiverWebSocketImpl&&) = default;

    void run()
    {
        m_session->run();
    }

    void poll()
    {
        m_session->poll(m_onReceive, m_onEvent);
    }

private:
    std::function<void(const dataslinger::message::Message&)> m_onReceive;
    std::function<void(const dataslinger::event::Event&)> m_onEvent;

    std::shared_ptr<DataReceiverWebSocketSession> m_session;
};

DataReceiverWebSocket::DataReceiverWebSocket(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionInfo& info)
    : d{std::make_unique<DataReceiverWebSocketImpl>(onReceive, onEvent, info)}
{
}

DataReceiverWebSocket::~DataReceiverWebSocket()
{
}

void DataReceiverWebSocket::run()
{
    d->run();
}

void DataReceiverWebSocket::poll()
{
    d->poll();
}

}
}
