#pragma once

#include <functional>
#include <memory>

#include "dataslinger/connection/connectioninfo.h"
#include "dataslinger/event/event.h"
#include "dataslinger/message/message.h"

namespace dataslinger
{

namespace pipe
{

/// Pipe backend responsible for receiving data
class DataReceiverPipe
{
public:
    DataReceiverPipe(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionInfo& info);
    ~DataReceiverPipe();
    DataReceiverPipe(const DataReceiverPipe&) = delete;
    DataReceiverPipe& operator=(const DataReceiverPipe&) = delete;
    DataReceiverPipe(DataReceiverPipe&&) = default;
    DataReceiverPipe& operator=(DataReceiverPipe&&) = default;

    /// Call once to set the receiver up
    void run();

    /// Enqueue a message ready to be sent
    void send(const dataslinger::message::Message& message);

    /// Poll the receiver to process new messages received or events that have occurred
    void poll();

private:
    class DataReceiverPipeImpl;
    std::unique_ptr<DataReceiverPipeImpl> d;
};

}

}
