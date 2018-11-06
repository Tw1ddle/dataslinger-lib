#include "dataslinger/websocket/datareceiverwebsocket.h"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "dataslinger/connection/connectioninfo.h"
#include "dataslinger/event/event.h"
#include "dataslinger/event/eventhelpers.h"
#include "dataslinger/message/message.h"

namespace
{

// WebSocket client
class DataReceiverWebSocketSession : public std::enable_shared_from_this<DataReceiverWebSocketSession>
{
public:
    DataReceiverWebSocketSession(const dataslinger::connection::ConnectionInfo& info) : m_resolver(m_ioContext), m_socketStream(m_ioContext), m_info{info}
    {
        m_socketStream.binary(true);
    }

    void run()
    {
        if(!m_info.hasWebSocketReceiverInfo()) {
            queueFatalEvent("Will fail to create receiver, missing connection info");
            return;
        }

        // Start the asynchronous operation - look up the domain name
        m_resolver.async_resolve(getHost(), std::to_string(getPort()),
            std::bind(&DataReceiverWebSocketSession::onResolve, shared_from_this(), std::placeholders::_1, std::placeholders::_2));

        m_ioContext.run();
    }

    void poll(const std::function<void(const dataslinger::message::Message&)>& onReceive,
              const std::function<void(const dataslinger::event::Event&)>& onEvent)
    {
        m_receiveQueue.consume_all([&onReceive](const dataslinger::message::Message m) {
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

        queueInformationalEvent("Did connect, will asynchronously send the websocket upgrade request (handshake)");

        m_socketStream.async_handshake(getHost(), "/", std::bind(&DataReceiverWebSocketSession::onHandshake, shared_from_this(), std::placeholders::_1));
    }

    void onHandshake(const boost::system::error_code ec)
    {
        if(ec) {
            queueFatalEvent(ec, "handshake");
            return;
        }

        queueInformationalEvent("Did receive websocket upgrade handshake response");

        doRead();
    }

    void doRead()
    {
        queueInformationalEvent("Will wait to receive a message");

        m_socketStream.async_read(m_receiveBuffer,
            std::bind(&DataReceiverWebSocketSession::onRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void onRead(const boost::system::error_code ec, const std::size_t bytesTransferred)
    {        
        if(ec) {
            queueFatalEvent(ec, "read");
            return;
        }

        queueInformationalEvent(std::string("Did perform read of ").append(std::to_string(bytesTransferred)).append(" bytes"));

        const dataslinger::message::Message* msg = reinterpret_cast<const dataslinger::message::Message*>(m_receiveBuffer.data().data());

        m_receiveQueue.push(*msg);

        queueInformationalEvent("Appended message to received queue, will clear intermediate buffer and continue to wait to receive messages");

        m_receiveBuffer.consume(m_receiveBuffer.size());

        doRead();
    }

    // Report a fatal error
    void queueFatalEvent(const boost::system::error_code ec, const std::string what)
    {
        const std::string msg = std::string(what).append("_").append(ec.message());
        m_eventQueue.push(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::RECEIVER, what));
    }
    void queueFatalEvent(const std::string what)
    {
        m_eventQueue.push(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::RECEIVER, what));
    }

    // Report an informational event
    void queueInformationalEvent(const std::string what)
    {
        m_eventQueue.push(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::RECEIVER, what));
    }

    std::string getHost() const
    {
        return m_info.getInfo().getValue<std::string>(dataslinger::connection::ConnectionInfoDataKeys::WEBSOCKET_RECEIVER_HOST_STRING);
    }

    std::uint16_t getPort() const
    {
         return m_info.getInfo().getValue<std::uint16_t>(dataslinger::connection::ConnectionInfoDataKeys::WEBSOCKET_RECEIVER_PORT_UINT16);
    }

    const dataslinger::connection::ConnectionInfo m_info; ///< Connection info

    boost::asio::io_context m_ioContext;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_socketStream;
    boost::beast::flat_buffer m_receiveBuffer; ///< Buffer that stores the most recently received WebSocket message

    // TODO ideally bound capacity by memory used or make unlimited
    boost::lockfree::spsc_queue<dataslinger::message::Message, boost::lockfree::fixed_sized<true>> m_receiveQueue{10000}; ///< Queue that grows as receiver receives messages
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
        : m_onReceive{onReceive}, m_onEvent{onEvent}, m_session{std::make_shared<DataReceiverWebSocketSession>(info)}
    {
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
    const std::function<void(const dataslinger::message::Message&)> m_onReceive;
    const std::function<void(const dataslinger::event::Event&)> m_onEvent;
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
