#include "dataslinger/backend/websocket/datareceiverwebsocket.h"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include "concurrentqueue.h"

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
    DataReceiverWebSocketSession(const dataslinger::connection::ConnectionInfo& info) : m_info{info}, m_resolver(m_ioContext), m_socketStream(m_ioContext), m_strand(m_socketStream.get_executor())
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
        dataslinger::message::Message m;
        while(m_receiveQueue.try_dequeue(m)) {
            onReceive(m);
        }
        dataslinger::event::Event e;
        while(m_eventQueue.try_dequeue(e)) {
            onEvent(e);
        }
    }

    void send(const dataslinger::message::Message& message)
    {
        const bool isEmpty = m_sendQueue.size_approx() == 0;

        m_sendQueue.enqueue(message);

        if(isEmpty) {
            boost::asio::post(m_socketStream.get_executor(), std::bind(&DataReceiverWebSocketSession::doWrite, this));
        }
    }

    void stop()
    {
        m_ioContext.stop();
    }

private:
    void onResolve(const boost::system::error_code ec, const boost::asio::ip::tcp::resolver::results_type results)
    {
        if(ec) {
            queueFatalEvent(ec, "Fatal error on resolve");
            return;
        }

        queueInformationalEvent("Will make the connection on the IP address we resolved from lookup");

        boost::asio::async_connect(m_socketStream.next_layer(), results.begin(), results.end(),
            std::bind(&DataReceiverWebSocketSession::onConnect, shared_from_this(), std::placeholders::_1));
    }

    void onConnect(const boost::system::error_code ec)
    {
        if(ec) {
            queueFatalEvent(ec, "Fatal error on connect");
            return;
        }

        queueInformationalEvent("Did connect, will asynchronously send the websocket upgrade request (handshake)");

        m_socketStream.async_handshake(getHost(), "/", std::bind(&DataReceiverWebSocketSession::onHandshake, shared_from_this(), std::placeholders::_1));
    }

    void onHandshake(const boost::system::error_code ec)
    {
        if(ec) {
            queueFatalEvent(ec, "Fatal error on handshake");
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

    void doWrite()
    {
        queueInformationalEvent("Will attempt to write a message");

        if(!m_sendQueue.try_dequeue(m_sendBuffer)) {
            return;
        }

        m_socketStream.async_write(boost::asio::const_buffer(reinterpret_cast<void*>(m_sendBuffer.data()), m_sendBuffer.size()),
            boost::asio::bind_executor(m_strand,
            std::bind(&DataReceiverWebSocketSession::onWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2)));
    }

    void onRead(const boost::system::error_code ec, const std::size_t bytesTransferred)
    {
        if(ec) {
            queueFatalEvent(ec, "Fatal error on read");
            return;
        }

        queueInformationalEvent(std::string("Did perform read of ").append(std::to_string(bytesTransferred)).append(" bytes"));

        const auto msgDataPtr = static_cast<const std::byte*>(m_receiveBuffer.data().data());
        m_receiveQueue.enqueue(dataslinger::message::Message(msgDataPtr, msgDataPtr + m_receiveBuffer.size()));

        queueInformationalEvent("Appended message to received queue, will clear intermediate buffer and continue to wait to receive messages");

        m_receiveBuffer.consume(m_receiveBuffer.size());

        doRead();
    }

    void onWrite(const boost::system::error_code ec, const std::size_t bytesTransferred)
    {
        if(ec) {
            queueFatalEvent(ec, "Fatal error on write");
            return;
        }

        queueInformationalEvent(std::string("Did perform write of ").append(std::to_string(bytesTransferred)).append(" bytes"));

        queueInformationalEvent("Did write, will continue to write messages as necessary");

        doWrite();
    }

    // Report a fatal error
    void queueFatalEvent(const boost::system::error_code ec, const std::string what)
    {
        const std::string msg = std::string(what).append("_").append(ec.message());
        m_eventQueue.enqueue(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::RECEIVER, what));
    }
    void queueFatalEvent(const std::string what)
    {
        m_eventQueue.enqueue(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::RECEIVER, what));
    }

    // Report an informational event
    void queueInformationalEvent(const std::string what)
    {
        m_eventQueue.enqueue(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::RECEIVER, what));
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
    boost::asio::strand<boost::asio::io_context::executor_type> m_strand;

    boost::beast::flat_buffer m_receiveBuffer; ///< Buffer that stores the most recently received WebSocket message
    std::vector<std::byte> m_sendBuffer;

    moodycamel::ConcurrentQueue<dataslinger::message::Message> m_sendQueue; ///< Queue that grows as messages are enqueued to be sent
    moodycamel::ConcurrentQueue<dataslinger::message::Message> m_receiveQueue; ///< Queue that grows as receiver receives messages
    moodycamel::ConcurrentQueue<dataslinger::event::Event> m_eventQueue; ///< Queue of internal events generated by the receiver
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

    void send(const dataslinger::message::Message& message)
    {
        m_session->send(message);
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

void DataReceiverWebSocket::send(const dataslinger::message::Message& message)
{
    d->send(message);
}

void DataReceiverWebSocket::poll()
{
    d->poll();
}

}
}
