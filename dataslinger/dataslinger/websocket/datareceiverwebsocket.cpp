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

// Sends a WebSocket message and prints the response
class DataReceiverWebSocketSession : public std::enable_shared_from_this<DataReceiverWebSocketSession>
{
public:
    DataReceiverWebSocketSession() : m_resolver(m_ioContext), m_socketStream(m_ioContext)
    {
        // Start the asynchronous operation
        // Save these for later
        m_port = "8081";
        m_host = "127.0.0.1";
        m_text = "todo";
    }

    void run()
    {
        // Look up the domain name
        m_resolver.async_resolve(m_host, m_port,
            std::bind(&DataReceiverWebSocketSession::onResolve, shared_from_this(), std::placeholders::_1, std::placeholders::_2));

        m_ioContext.run();
    }

private:    
    void onResolve(boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results)
    {
        if(ec) {
            return fail(ec, "resolve");
        }

        // Make the connection on the IP address we get from a lookup
        boost::asio::async_connect(m_socketStream.next_layer(), results.begin(), results.end(),
            std::bind(&DataReceiverWebSocketSession::onConnect, shared_from_this(), std::placeholders::_1));
    }

    void onConnect(boost::system::error_code ec)
    {
        if(ec) {
            return fail(ec, "connect");
        }

        // Perform the websocket handshake
        m_socketStream.async_handshake(m_host, "/", std::bind(&DataReceiverWebSocketSession::onHandshake, shared_from_this(), std::placeholders::_1));
    }

    void onHandshake(boost::system::error_code ec)
    {
        if(ec) {
            return fail(ec, "handshake");
        }

        // Send the message
        m_socketStream.async_write(boost::asio::buffer(m_text),
            std::bind(&DataReceiverWebSocketSession::onWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void onWrite(boost::system::error_code ec, std::size_t bytesTransferred)
    {
        boost::ignore_unused(bytesTransferred);

        if(ec) {
            return fail(ec, "write");
        }

        // Read a message into our buffer
        m_socketStream.async_read(m_buffer,
            std::bind(&DataReceiverWebSocketSession::onRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void onRead(boost::system::error_code ec, std::size_t bytesTransferred)
    {
        boost::ignore_unused(bytesTransferred);

        if(ec) {
            return fail(ec, "read");
        }

        // Close the WebSocket connection
        m_socketStream.async_close(boost::beast::websocket::close_code::normal,
            std::bind(&DataReceiverWebSocketSession::onClose, shared_from_this(), std::placeholders::_1));
    }

    void onClose(boost::system::error_code ec)
    {
        if(ec) {
            return fail(ec, "close");
        }

        // If we get here then the connection is closed gracefully

        // The buffers() function helps print a ConstBufferSequence
        //std::cout << boost::beast::buffers(m_buffer.data()) << std::endl;
    }

    boost::asio::io_context m_ioContext;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_socketStream;
    boost::beast::multi_buffer m_buffer;
    std::string m_host;
    std::string m_port;
    std::string m_text;
};

}

namespace dataslinger
{
namespace websocket
{

class DataReceiverWebSocket::DataReceiverWebSocketImpl
{
public:
    DataReceiverWebSocketImpl(DataReceiverWebSocket* wsReceiver, DataReceiver* receiver) : m_receiver{receiver}, m_receiverWs{wsReceiver}
    {
        m_session = std::make_shared<DataReceiverWebSocketSession>();
        m_session->run();
    }

    ~DataReceiverWebSocketImpl()
    {

    }

    DataReceiverWebSocketImpl(const DataReceiverWebSocketImpl&) = delete;
    DataReceiverWebSocketImpl& operator=(const DataReceiverWebSocketImpl&) = delete;
    DataReceiverWebSocketImpl(DataReceiverWebSocketImpl&&) = default;
    DataReceiverWebSocketImpl& operator=(DataReceiverWebSocketImpl&&) = default;

private:
    DataReceiver* m_receiver;
    DataReceiverWebSocket* m_receiverWs;

    std::shared_ptr<DataReceiverWebSocketSession> m_session;
};

DataReceiverWebSocket::DataReceiverWebSocket(DataReceiver* q) : d{std::make_unique<DataReceiverWebSocketImpl>(this, q)}
{
}

DataReceiverWebSocket::~DataReceiverWebSocket()
{
}

}
}
