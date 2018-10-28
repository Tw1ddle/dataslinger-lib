#include "dataslinger/datareceiver.h"

#include <cstdlib>
#include <memory>

#include <boost/scope_exit.hpp>

#include <chaiscript/chaiscript.hpp>

#include "dataslinger/websocket/datareceiverwebsocket.h"

namespace dataslinger
{

class DataReceiver::DataReceiverImpl
{
public:
    DataReceiverImpl(DataReceiver* pQ) : q{pQ}, d{std::make_unique<websocket::DataReceiverWebSocket>(q)}
    {
    }

    ~DataReceiverImpl()
    {
    }

    DataReceiverImpl(const DataReceiverImpl&) = delete;
    DataReceiverImpl& operator=(const DataReceiverImpl&) = delete;
    DataReceiverImpl(DataReceiverImpl&&) = default;
    DataReceiverImpl& operator=(DataReceiverImpl&&) = default;

    void receive()
    {
        BOOST_SCOPE_EXIT(this_) {
            this_->q->signal_afterReceive();
        } BOOST_SCOPE_EXIT_END

        q->signal_beforeReceive();

        // TODO hook into engine
    }

private:
    DataReceiver* q;
    std::unique_ptr<websocket::DataReceiverWebSocket> d;

    chaiscript::ChaiScript m_engine;
};

DataReceiver::DataReceiver() : d{std::make_unique<DataReceiver::DataReceiverImpl>(this)}
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

}
