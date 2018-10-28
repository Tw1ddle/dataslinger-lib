#pragma once

#include <memory>

#include "dataslinger/datareceiver.h"

namespace dataslinger
{

namespace websocket
{

/// WebSocket backend responsible for receiving data
class DataReceiverWebSocket
{
public:
    DataReceiverWebSocket(DataReceiver* q);
    ~DataReceiverWebSocket();
    DataReceiverWebSocket(const DataReceiverWebSocket&) = delete;
    DataReceiverWebSocket& operator=(const DataReceiverWebSocket&) = delete;
    DataReceiverWebSocket(DataReceiverWebSocket&&) = default;
    DataReceiverWebSocket& operator=(DataReceiverWebSocket&&) = default;

private:
    class DataReceiverWebSocketImpl;
    std::unique_ptr<DataReceiverWebSocketImpl> d;
};

}

}
