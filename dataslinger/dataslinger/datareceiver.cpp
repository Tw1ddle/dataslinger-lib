#include "dataslinger/datareceiver.h"

#include <cstdlib>
#include <functional>
#include <future>
#include <memory>

#include "dataslinger/connection/connectioninfo.h"
#include "dataslinger/event/event.h"
#include "dataslinger/message/message.h"
#include "dataslinger/websocket/datareceiverwebsocket.h"

namespace dataslinger
{

class DataReceiver::DataReceiverImpl
{
public:
    DataReceiverImpl(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionInfo& info)
        : d{std::make_unique<websocket::DataReceiverWebSocket>(onReceive, onEvent, info)}
    {
        m_receiverFuture = std::async(std::launch::async, [this]{
            d->run();
        });
    }

    ~DataReceiverImpl()
    {
        d->poll();
    }

    DataReceiverImpl(const DataReceiverImpl&) = delete;
    DataReceiverImpl& operator=(const DataReceiverImpl&) = delete;
    DataReceiverImpl(DataReceiverImpl&&) = default;
    DataReceiverImpl& operator=(DataReceiverImpl&&) = default;

    void poll()
    {
        d->poll();
    }

private:
    std::future<void> m_receiverFuture;
    std::unique_ptr<websocket::DataReceiverWebSocket> d;
};

DataReceiver::DataReceiver(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionInfo& info) : d{std::make_unique<DataReceiver::DataReceiverImpl>(onReceive, onEvent, info)}
{
}

DataReceiver::~DataReceiver()
{
}

DataReceiver::DataReceiver(DataReceiver&& other) : d{std::move(other.d)}
{
    other.d = nullptr;
}

DataReceiver& DataReceiver::operator=(DataReceiver&& other)
{
    // Self-assignment detection
    if (&other == this) {
        return *this;
    }
    d = std::move(other.d);
    other.d = nullptr;
    return *this;
}

void DataReceiver::poll()
{
    d->poll();
}

}
