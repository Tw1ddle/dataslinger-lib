#include "dataslinger/websocket/dataslingerwebsocket.h"

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
#include <boost/scope_exit.hpp>

#include "dataslinger/dataslingererrors.h"

namespace
{

// Report a failure
void fail(boost::system::error_code ec, std::string what)
{
    throw dataslinger::error::DataSlingerError({{
        { dataslinger::error::ErrorDataKeys::MESSAGE_STRING, std::string(what.append("_").append(ec.message())) }
    }});
}

// Echoes back all received websocket messages
class DataSlingerWebSocketSession : public std::enable_shared_from_this<DataSlingerWebSocketSession>
{
public:
    explicit DataSlingerWebSocketSession(boost::asio::ip::tcp::socket socket) : m_socketStream(std::move(socket)), m_strand(m_socketStream.get_executor())
    {
        // Start the asynchronous operation, accept the websocket handshake
        m_socketStream.async_accept(boost::asio::bind_executor(m_strand,
            std::bind(&DataSlingerWebSocketSession::onAccept, shared_from_this(), std::placeholders::_1)));
    }

private:
    void onAccept(boost::system::error_code ec)
    {
        if(ec) {
            return fail(ec, "accept");
        }

        // Read a message
        doRead();
    }

    void doRead()
    {
        // Read a message into our buffer
        m_socketStream.async_read(
            m_buffer,
            boost::asio::bind_executor(m_strand,
            std::bind(&DataSlingerWebSocketSession::onRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2)));
    }

    void onRead(boost::system::error_code ec, std::size_t /*bytesTransferred*/)
    {
        // This indicates that the session was closed
        if(ec == boost::beast::websocket::error::closed) {
            return;
        }

        if(ec) {
            fail(ec, "read");
        }

        // Echo the message
        m_socketStream.text(m_socketStream.got_text());
        m_socketStream.async_write(m_buffer.data(),
            boost::asio::bind_executor(m_strand,
            std::bind(&DataSlingerWebSocketSession::onWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2)));
    }

    void onWrite(boost::system::error_code ec, std::size_t /*bytesTransferred*/)
    {
        if(ec) {
            return fail(ec, "write");
        }

        // Clear the buffer
        m_buffer.consume(m_buffer.size());

        // Do another read
        doRead();
    }

    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_socketStream;
    boost::asio::strand<boost::asio::io_context::executor_type> m_strand;
    boost::beast::multi_buffer m_buffer;
};

// Accepts incoming connections and launches sessions
class DataSlingerWebSocketListener : public std::enable_shared_from_this<DataSlingerWebSocketListener>
{
public:
    DataSlingerWebSocketListener(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint endpoint) : m_acceptor(ioc), m_socket(ioc)
    {
        boost::system::error_code ec;

        // Open the acceptor
        m_acceptor.open(endpoint.protocol(), ec);
        if(ec) {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        m_acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if(ec) {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        m_acceptor.bind(endpoint, ec);
        if(ec) {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);

        if(ec) {
            fail(ec, "listen");
            return;
        }

        // Start accepting incoming connections
        if(!m_acceptor.is_open()) {
            return;
        }
        doAccept();
    }

private:
    void doAccept()
    {
        m_acceptor.async_accept(m_socket, std::bind(&DataSlingerWebSocketListener::onAccept, shared_from_this(), std::placeholders::_1));
    }

    void onAccept(boost::system::error_code ec)
    {
        if(ec) {
            fail(ec, "accept");
        } else {
            // Create the session and run it
            std::make_shared<DataSlingerWebSocketSession>(std::move(m_socket));
        }

        // Accept another connection
        doAccept();
    }

    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;
};

}

namespace dataslinger
{
namespace websocket
{

class DataSlingerWebSocket::DataSlingerWebSocketImpl
{
public:
    DataSlingerWebSocketImpl(DataSlingerWebSocket* slingerWebSocket, DataSlinger* slinger) : m_slinger{slinger}, m_slingerWebSocket{slingerWebSocket}
    {
        //TODO
        //boost::asio::io_context ioc{1};
        //std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();
        // Run the I/O service on the requested number of threads
        //std::vector<std::thread> v;
        //v.reserve(threads - 1);
        //for(auto i = threads - 1; i > 0; --i)
        //    v.emplace_back(
        //    [&ioc]
        //    {
        //        ioc.run();
        //    });
        //ioc.run();
    }

    ~DataSlingerWebSocketImpl()
    {
    }

    DataSlingerWebSocketImpl(const DataSlingerWebSocketImpl&) = delete;
    DataSlingerWebSocketImpl& operator=(const DataSlingerWebSocketImpl&) = delete;
    DataSlingerWebSocketImpl(DataSlingerWebSocketImpl&&) = default;
    DataSlingerWebSocketImpl& operator=(DataSlingerWebSocketImpl&&) = default;

private:
    DataSlinger* m_slinger;
    DataSlingerWebSocket* m_slingerWebSocket;

    //DataSlingerWebSocketListener m_listener;
};

DataSlingerWebSocket::DataSlingerWebSocket(DataSlinger* q) : d{std::make_unique<DataSlingerWebSocketImpl>(this, q)}
{
}

DataSlingerWebSocket::~DataSlingerWebSocket()
{
}

}
}
