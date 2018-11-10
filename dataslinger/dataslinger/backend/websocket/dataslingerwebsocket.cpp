#include "dataslinger/backend/websocket/dataslingerwebsocket.h"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include "concurrentqueue.h"

#include "dataslinger/connection/connectionoptions.h"
#include "dataslinger/event/event.h"
#include "dataslinger/event/eventhelpers.h"
#include "dataslinger/message/message.h"

namespace
{

// Represents a session
class DataSlingerWebSocketSession : public std::enable_shared_from_this<DataSlingerWebSocketSession>
{
public:
    explicit DataSlingerWebSocketSession(boost::asio::ip::tcp::socket socket) : m_socketStream(std::move(socket)), m_strand(m_socketStream.get_executor())
    {
        m_socketStream.binary(true);
    }

    void run()
    {
        queueInformationalEvent("Will asynchronously wait to accept the websocket upgrade request");

        m_socketStream.async_accept(boost::asio::bind_executor(m_strand,
            std::bind(&DataSlingerWebSocketSession::onAccept, shared_from_this(), std::placeholders::_1)));
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
            boost::asio::post(m_socketStream.get_executor(), std::bind(&DataSlingerWebSocketSession::doWrite, this));
        }
    }

private:
    void onAccept(const boost::system::error_code ec)
    {
        if(ec) {
            queueFatalEvent(ec, "Fatal error on accept");
            return;
        }

        queueInformationalEvent("Did accept websocket upgrade request");

        doRead();
    }

    void doRead()
    {
        queueInformationalEvent("Will wait to receive a message");

        m_socketStream.async_read(m_receiveBuffer,
            std::bind(&DataSlingerWebSocketSession::onRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void doWrite()
    {
        queueInformationalEvent("Will attempt to write a message");

        if(!m_sendQueue.try_dequeue(m_sendBuffer)) {
            return;
        }

        m_socketStream.async_write(boost::asio::const_buffer(reinterpret_cast<void*>(m_sendBuffer.data()), m_sendBuffer.size()),
            boost::asio::bind_executor(m_strand,
            std::bind(&DataSlingerWebSocketSession::onWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2)));
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
            queueFatalEvent(ec, "write");
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

        m_eventQueue.enqueue(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::SLINGER, what));
    }

    void queueFatalEvent(const std::string what)
    {
        m_eventQueue.enqueue(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::SLINGER, what));
    }

    // Report an informational event
    void queueInformationalEvent(const std::string what)
    {
        m_eventQueue.enqueue(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::SLINGER, what));
    }

    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_socketStream;
    boost::asio::strand<boost::asio::io_context::executor_type> m_strand;
    boost::beast::flat_buffer m_receiveBuffer;
    std::vector<std::byte> m_sendBuffer;

    moodycamel::ConcurrentQueue<dataslinger::message::Message> m_sendQueue; ///< Queue that grows as messages are enqueued to be sent
    moodycamel::ConcurrentQueue<dataslinger::message::Message> m_receiveQueue; ///< Queue that grows as slinger receives messages
    moodycamel::ConcurrentQueue<dataslinger::event::Event> m_eventQueue; ///< Queue of internal events generated by the session
};

// Accepts incoming connections and launches sessions
class DataSlingerWebSocketListener : public std::enable_shared_from_this<DataSlingerWebSocketListener>
{
public:
    DataSlingerWebSocketListener(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint endpoint) : m_acceptor(ioc), m_socket(ioc), m_endpoint(endpoint)
    {
    }

    void run()
    {
        boost::system::error_code ec;

        queueInformationalEvent("Will open the acceptor");

        m_acceptor.open(m_endpoint.protocol(), ec);
        if(ec) {
            queueFatalEvent(ec, "Fatal error on opening acceptor");
            return;
        }

        queueInformationalEvent("Will allow address reuse");

        m_acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if(ec) {
            queueFatalEvent(ec, "Fatal error on setting up address reuse");
            return;
        }

        queueInformationalEvent("Will bind to the server address");

        m_acceptor.bind(m_endpoint, ec);
        if(ec) {
            queueFatalEvent(ec, "Fatal error on binding to server address");
            return;
        }

        queueInformationalEvent("Will start listening for connections");

        m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
        if(ec) {
            queueFatalEvent(ec, "Fatal error on starting listening");
            return;
        }

        queueInformationalEvent("Will check if acceptor is open");

        if(!m_acceptor.is_open()) {
            queueFatalEvent("Fatal error, acceptor should be open but wasn't");
            return;
        }

        doAccept();
    }

    void poll(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent)
    {
        dataslinger::event::Event e;
        while(m_eventQueue.try_dequeue(e)) {
            onEvent(e);
        }

        std::lock_guard<std::mutex> m(m_sessionsMutex);
        for(auto& session : m_sessions) {
            session->poll(onReceive, onEvent);
        }
    }

    void send(const dataslinger::message::Message& message)
    {
        for(auto& session : m_sessions) {
            session->send(message);
        }
    }

private:
    void doAccept()
    {
        queueInformationalEvent("Will start accepting incoming connections");

        m_acceptor.async_accept(m_socket, std::bind(&DataSlingerWebSocketListener::onAccept, shared_from_this(), std::placeholders::_1));
    }

    void onAccept(const boost::system::error_code ec)
    {
        queueInformationalEvent("Did accept incoming connection");

        if(ec) {
            queueFatalEvent(ec, "Fatal error on accept");
            return;
        } else {
            queueInformationalEvent("Will create new session");

            std::lock_guard<std::mutex> m(m_sessionsMutex);
            m_sessions.emplace_back(std::make_shared<DataSlingerWebSocketSession>(std::move(m_socket)));
            m_sessions.back()->run();
        }

        queueInformationalEvent("Will continue to accept incoming connections");

        doAccept();
    }

    // Report a fatal event
    void queueFatalEvent(const boost::system::error_code ec, const std::string what)
    {
        const std::string msg = std::string(what).append("_").append(ec.message());
        m_eventQueue.enqueue(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::SLINGER, what));
    }

    void queueFatalEvent(const std::string what)
    {
        m_eventQueue.enqueue(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::SLINGER, what));
    }

    // Report an informational event
    void queueInformationalEvent(const std::string what)
    {
        m_eventQueue.enqueue(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::SLINGER, what));
    }

    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::endpoint m_endpoint;

    std::vector<std::shared_ptr<DataSlingerWebSocketSession>> m_sessions;
    std::mutex m_sessionsMutex;

    moodycamel::ConcurrentQueue<dataslinger::event::Event> m_eventQueue; ///< Queue of internal events generated by the listener
};

}

namespace dataslinger
{
namespace websocket
{

// WebSocket server implementation
class DataSlingerWebSocket::DataSlingerWebSocketImpl
{
public:
    DataSlingerWebSocketImpl(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info) : m_onReceive{onReceive}, m_onEvent{onEvent}, m_info{info}
    {
    }

    ~DataSlingerWebSocketImpl()
    {
        stop();
    }

    DataSlingerWebSocketImpl(const DataSlingerWebSocketImpl&) = delete;
    DataSlingerWebSocketImpl& operator=(const DataSlingerWebSocketImpl&) = delete;
    DataSlingerWebSocketImpl(DataSlingerWebSocketImpl&&) = default;
    DataSlingerWebSocketImpl& operator=(DataSlingerWebSocketImpl&&) = default;

    void run()
    {
        queueInformationalEvent("Will set up data slinger");

        if(!m_info.hasWebSocketInfo()) {
            queueFatalEvent("Will fail to set up slinger, incomplete connection info");
            return;
        }

        const std::string host = m_info.getInfo().getValue<std::string>(dataslinger::connection::ConnectionOption::WEBSOCKET_HOST_STRING);
        const std::uint16_t port = m_info.getInfo().getValue<std::uint16_t>(dataslinger::connection::ConnectionOption::WEBSOCKET_PORT_UINT16);

        const boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::address::from_string(host), port};
        m_ioc = std::make_shared<boost::asio::io_context>(1);

        m_listener = std::make_shared<DataSlingerWebSocketListener>(*m_ioc.get(), endpoint);
        m_listener->run();

        // Capture SIGINT and SIGTERM for clean shutdown
        boost::asio::signal_set sigs(*m_ioc.get(), SIGINT, SIGTERM);
        sigs.async_wait([&](boost::system::error_code const&, int) {
            // This will cause run() to return immediately, eventually destroying the context and sockets in it
            m_ioc->stop();
        });

        // Run the I/O service on the requested number of threads
        const std::size_t threads = 4;
        std::vector<std::thread> v;
        const std::shared_ptr<boost::asio::io_context> ioc{m_ioc};
        v.reserve(threads);
        for(auto i = threads - 1; i > 0; --i) {
            v.emplace_back([ioc] { ioc->run(); });
        }
        m_ioc->run();

        for(auto& t : v) {
            t.join();
        }
    }

    void send(const dataslinger::message::Message& message)
    {
        if(m_listener) {
            m_listener->send(message);
        }
    }

    void poll()
    {
        dataslinger::event::Event e;
        while(m_eventQueue.try_dequeue(e)) {
            m_onEvent(e);
        }

        if(m_listener) {
            m_listener->poll(m_onReceive, m_onEvent);
        }
    }

    void stop()
    {
        queueInformationalEvent("Data slinger is closing down, will block until all the I/O service threads exit");

        if(m_ioc) {
            m_ioc->stop();
        }
    }

private:
    void queueFatalEvent(const std::string what)
    {
        m_eventQueue.enqueue(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::SLINGER, what));
    }

    // Report an informational event
    void queueInformationalEvent(const std::string what)
    {
        m_eventQueue.enqueue(dataslinger::event::makeEvent(dataslinger::event::EventSourceKind::SLINGER, what));
    }

    const std::function<void(const dataslinger::message::Message&)> m_onReceive; ///< Callback triggered when the slinger receives a message
    const std::function<void(const dataslinger::event::Event&)> m_onEvent; ///< Callback triggered when the slinger produces an event
    const dataslinger::connection::ConnectionOptions m_info; ///< Connection info

    std::shared_ptr<boost::asio::io_context> m_ioc{nullptr}; ///< The IO context
    std::shared_ptr<DataSlingerWebSocketListener> m_listener{nullptr}; ///< The listener that allows receivers to connect to the slinger

    moodycamel::ConcurrentQueue<dataslinger::event::Event> m_eventQueue; ///< Queue of internal events generated by the slinger
};

DataSlingerWebSocket::DataSlingerWebSocket(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info) : d{std::make_unique<DataSlingerWebSocketImpl>(onReceive, onEvent, info)}
{
}

DataSlingerWebSocket::~DataSlingerWebSocket()
{
    d->stop();
}

void DataSlingerWebSocket::run()
{
    d->run();
}

void DataSlingerWebSocket::send(const dataslinger::message::Message& message)
{
    d->send(message);
}

void DataSlingerWebSocket::poll()
{
    d->poll();
}

}
}
