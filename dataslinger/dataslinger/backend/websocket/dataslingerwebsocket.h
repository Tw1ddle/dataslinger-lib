#pragma once

#include <functional>
#include <memory>

#include "dataslinger/connection/connectionoptions.h"
#include "dataslinger/event/event.h"
#include "dataslinger/message/message.h"

namespace dataslinger
{
	
namespace websocket
{

/// WebSocket backend responsible for sending data
class DataSlingerWebSocket
{
public:
    DataSlingerWebSocket(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info);
    ~DataSlingerWebSocket();
    DataSlingerWebSocket(const DataSlingerWebSocket&) = delete;
    DataSlingerWebSocket& operator=(const DataSlingerWebSocket&) = delete;
    DataSlingerWebSocket(DataSlingerWebSocket&&) = default;
    DataSlingerWebSocket& operator=(DataSlingerWebSocket&&) = default;

    /// Call once to set the slinger up
    void run();

    /// Enqueue a message ready to be sent
    void send(const dataslinger::message::Message& message);

    /// Poll the slinger to send enqueued messages, process received messages, and handle errors/events that have occurred
    void poll();

private:
    class DataSlingerWebSocketImpl;
    std::unique_ptr<DataSlingerWebSocketImpl> d;
};

}	

}
