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

/// Pipe backend responsible for sending data
class DataSlingerPipe
{
public:
    DataSlingerPipe(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionInfo& info);
    ~DataSlingerPipe();
    DataSlingerPipe(const DataSlingerPipe&) = delete;
    DataSlingerPipe& operator=(const DataSlingerPipe&) = delete;
    DataSlingerPipe(DataSlingerPipe&&) = default;
    DataSlingerPipe& operator=(DataSlingerPipe&&) = default;

    /// Call once to set the slinger up
    void run();

    /// Enqueue a message ready to be sent
    void send(const dataslinger::message::Message& message);

    /// Poll the slinger to send enqueued messages, process received messages, and handle errors/events that have occurred
    void poll();

private:
    class DataSlingerPipeImpl;
    std::unique_ptr<DataSlingerPipeImpl> d;
};

}	

}
