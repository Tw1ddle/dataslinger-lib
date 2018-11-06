#include "dataslinger/dataslinger.h"

#include <cstdlib>
#include <future>
#include <memory>

#include "dataslinger/connection/connectioninfo.h"
#include "dataslinger/event/event.h"
#include "dataslinger/message/message.h"
#include "dataslinger/websocket/dataslingerwebsocket.h"

namespace dataslinger
{

class DataSlinger::DataSlingerImpl
{
public:
    DataSlingerImpl(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionInfo& info) : d{std::make_unique<websocket::DataSlingerWebSocket>(onReceive, onEvent, info)}
    {
        m_slingerFuture = std::async(std::launch::async, [this]{
            d->run();
        });
    }

    ~DataSlingerImpl()
    {
        poll();
    }

    DataSlingerImpl(const DataSlingerImpl&) = delete;
    DataSlingerImpl& operator=(const DataSlingerImpl&) = delete;
    DataSlingerImpl(DataSlingerImpl&&) = default;
    DataSlingerImpl& operator=(DataSlingerImpl&&) = default;

    void send(const dataslinger::message::Message& message)
    {
        d->send(message);
    }

    void poll()
    {
        d->poll();
    }

private:
    std::future<void> m_slingerFuture;
    std::unique_ptr<websocket::DataSlingerWebSocket> d;
};

DataSlinger::DataSlinger(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionInfo& info) : d{std::make_unique<DataSlinger::DataSlingerImpl>(onReceive, onEvent, info)}
{
}

DataSlinger::~DataSlinger()
{
}

DataSlinger::DataSlinger(DataSlinger&& other) : d{std::move(other.d)}
{
    other.d = nullptr;
}

DataSlinger& DataSlinger::operator=(DataSlinger&& other)
{
    // Self-assignment detection
    if (&other == this) {
        return *this;
    }
    d = std::move(other.d);
    other.d = nullptr;
    return *this;
}

void DataSlinger::send(const dataslinger::message::Message& message)
{
    d->send(message);
}

void DataSlinger::poll()
{
    d->poll();
}

}
