#pragma once

#include <memory>

#include "dataslinger/dataslinger.h"

namespace dataslinger
{
	
namespace websocket
{

/// WebSocket backend responsible for sending data
class DataSlingerWebSocket
{
public:
    DataSlingerWebSocket(DataSlinger* q);
    ~DataSlingerWebSocket();
    DataSlingerWebSocket(const DataSlingerWebSocket&) = delete;
    DataSlingerWebSocket& operator=(const DataSlingerWebSocket&) = delete;
    DataSlingerWebSocket(DataSlingerWebSocket&&) = default;
    DataSlingerWebSocket& operator=(DataSlingerWebSocket&&) = default;

private:
    class DataSlingerWebSocketImpl;
    std::unique_ptr<DataSlingerWebSocketImpl> d;
};

}	

}
