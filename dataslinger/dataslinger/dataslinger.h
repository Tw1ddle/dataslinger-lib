#pragma once

#include <functional>
#include <memory>

#include "dataslinger/connection/connectioninfo.h"
#include "dataslinger/event/event.h"
#include "dataslinger/message/message.h"

namespace dataslinger
{

/// Responsible for sending data
class DataSlinger
{
public:
    DataSlinger(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionInfo& info);
    ~DataSlinger();
    DataSlinger(const DataSlinger&) = delete;
    DataSlinger& operator=(const DataSlinger&) = delete;
    DataSlinger(DataSlinger&&);
    DataSlinger& operator=(DataSlinger&&);

    /// Sends a message to any connected receivers (enqueues for sending in some cases)
    void send(const dataslinger::message::Message& message);

    /// Poll to process responses from any connected receiver sessions and handle any events that have occurred
    void poll();

private:
    class DataSlingerImpl;
    std::unique_ptr<DataSlingerImpl> d;
};

}
