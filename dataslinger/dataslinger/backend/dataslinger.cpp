#include "dataslinger/slinger.h"

#include <cstdlib>
#include <future>
#include <memory>

#include "dataslinger/backend/factory/dataslingerfactory.h"

namespace dataslinger
{

class DataSlinger::DataSlingerImpl
{
public:
    DataSlingerImpl(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info) : m_backend{dataslinger::factory::makeSlingerBackend(onReceive, onEvent, info)}
    {
        m_slingerFuture = std::async(std::launch::async, [this] {
            m_backend.run();
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
        m_backend.send(message);
    }

    void poll()
    {
        m_backend.poll();
    }

private:
    std::future<void> m_slingerFuture;
    dataslinger::factory::Backend m_backend;
};

DataSlinger::DataSlinger(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info) : d{std::make_unique<DataSlinger::DataSlingerImpl>(onReceive, onEvent, info)}
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
