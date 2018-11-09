#pragma once

#include <functional>
#include <memory>

#include "dataslinger/connection/connectioninfo.h"
#include "dataslinger/event/event.h"
#include "dataslinger/message/message.h"

namespace dataslinger
{

namespace websocket
{

/// WebSocket backend responsible for receiving data
class DataReceiverWebSocket
{
public:
    DataReceiverWebSocket(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionInfo& info);
    ~DataReceiverWebSocket();
    DataReceiverWebSocket(const DataReceiverWebSocket&) = delete;
    DataReceiverWebSocket& operator=(const DataReceiverWebSocket&) = delete;
    DataReceiverWebSocket(DataReceiverWebSocket&&) = default;
    DataReceiverWebSocket& operator=(DataReceiverWebSocket&&) = default;

    /// Call once to set the receiver up
    void run();

    /// Enqueue a message ready to be sent
    void send(const dataslinger::message::Message& message);

    /// Poll the receiver to process new messages received or events that have occurred
    void poll();

private:
    class DataReceiverWebSocketImpl;
    std::unique_ptr<DataReceiverWebSocketImpl> d;
};

}

}
