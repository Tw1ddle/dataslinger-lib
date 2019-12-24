#include "dataslinger/backend/pipe/dataslingerpipe.h"

#include <functional>

#include "dataslinger/slinger.h"
#include "dataslinger/event/eventhelpers.h"

namespace dataslinger::pipe
{

class DataSlingerPipe::DataSlingerPipeImpl
{
public:
    DataSlingerPipeImpl(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info) : m_onReceive{onReceive}, m_onEvent{onEvent}, m_info{info}
    {
    }

    ~DataSlingerPipeImpl()
    {
    }

    DataSlingerPipeImpl(const DataSlingerPipeImpl&) = delete;
    DataSlingerPipeImpl& operator=(const DataSlingerPipeImpl&) = delete;
    DataSlingerPipeImpl(DataSlingerPipeImpl&&) = default;
    DataSlingerPipeImpl& operator=(DataSlingerPipeImpl&&) = default;

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
    const std::function<void(const dataslinger::message::Message&)> m_onReceive; ///< Callback triggered when the slinger receives a message
    const std::function<void(const dataslinger::event::Event&)> m_onEvent; ///< Callback triggered when the slinger produces an event
    const dataslinger::connection::ConnectionOptions m_info; ///< Connection info
};

DataSlingerPipe::DataSlingerPipe(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info) : d{std::make_unique<DataSlingerPipeImpl>(onReceive, onEvent, info)}
{
}

DataSlingerPipe::~DataSlingerPipe()
{
}

void DataSlingerPipe::run()
{
    d->run();
}

void DataSlingerPipe::send(const dataslinger::message::Message& message)
{
    d->send(message);
}

void DataSlingerPipe::poll()
{
    d->poll();
}

}
