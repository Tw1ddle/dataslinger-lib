#include "dataslinger/backend/pipe/datareceiverpipe.h"

#include <functional>

#include "dataslinger/connection/connectionoptions.h"
#include "dataslinger/event/event.h"
#include "dataslinger/event/eventhelpers.h"
#include "dataslinger/message/message.h"

namespace dataslinger
{
namespace pipe
{

class DataReceiverPipe::DataReceiverPipeImpl
{
public:
    DataReceiverPipeImpl(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info)
        : m_onReceive{onReceive}, m_onEvent{onEvent}
    {
    }

    ~DataReceiverPipeImpl() = default;
    DataReceiverPipeImpl(const DataReceiverPipeImpl&) = delete;
    DataReceiverPipeImpl& operator=(const DataReceiverPipeImpl&) = delete;
    DataReceiverPipeImpl(DataReceiverPipeImpl&&) = default;
    DataReceiverPipeImpl& operator=(DataReceiverPipeImpl&&) = default;

    void run()
    {

    }

    void send(const dataslinger::message::Message& /*message*/)
    {

    }

    void poll()
    {

    }

private:
    const std::function<void(const dataslinger::message::Message&)> m_onReceive;
    const std::function<void(const dataslinger::event::Event&)> m_onEvent;
};

DataReceiverPipe::DataReceiverPipe(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info)
    : d{std::make_unique<DataReceiverPipeImpl>(onReceive, onEvent, info)}
{
}

DataReceiverPipe::~DataReceiverPipe()
{
}

void DataReceiverPipe::run()
{
    d->run();
}

void DataReceiverPipe::send(const dataslinger::message::Message& message)
{
    d->send(message);
}

void DataReceiverPipe::poll()
{
    d->poll();
}

}
}
