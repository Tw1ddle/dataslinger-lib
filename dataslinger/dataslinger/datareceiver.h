#pragma once

#include <functional>
#include <memory>

#include "dataslinger/connection/connectioninfo.h"
#include "dataslinger/event/event.h"
#include "dataslinger/message/message.h"

namespace dataslinger
{

/// Responsible for receiving data
class DataReceiver
{
public:
    DataReceiver(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionInfo& info);
    ~DataReceiver();
    DataReceiver(const DataReceiver&) = delete;
    DataReceiver& operator=(const DataReceiver&) = delete;
    DataReceiver(DataReceiver&&);
    DataReceiver& operator=(DataReceiver&&);

    /// Poll to process new messages that have been received and events that have occurred
    void poll();

private:
    class DataReceiverImpl;
    std::unique_ptr<DataReceiverImpl> d;
};

}
