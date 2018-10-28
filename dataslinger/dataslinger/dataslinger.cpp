#include "dataslinger/dataslinger.h"

#include <cstdlib>
#include <memory>

#include <boost/scope_exit.hpp>

#include <chaiscript/chaiscript.hpp>

#include "dataslinger/websocket/dataslingerwebsocket.h"

namespace dataslinger
{

class DataSlinger::DataSlingerImpl
{
public:
    DataSlingerImpl(DataSlinger* pQ) : q{pQ}, d{std::make_unique<websocket::DataSlingerWebSocket>(pQ)}
    {
    }

    ~DataSlingerImpl()
    {
    }

    DataSlingerImpl(const DataSlingerImpl&) = delete;
    DataSlingerImpl& operator=(const DataSlingerImpl&) = delete;
    DataSlingerImpl(DataSlingerImpl&&) = default;
    DataSlingerImpl& operator=(DataSlingerImpl&&) = default;

    void send()
    {
        BOOST_SCOPE_EXIT(this_) {
            this_->q->signal_afterSend();
        } BOOST_SCOPE_EXIT_END

        q->signal_beforeSend();

        // TODO hook into engine
    }

private:
    DataSlinger* q;
    std::unique_ptr<websocket::DataSlingerWebSocket> d;

    chaiscript::ChaiScript m_engine;
};

DataSlinger::DataSlinger() : d{std::make_unique<DataSlinger::DataSlingerImpl>(this)}
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

void DataSlinger::send()
{
    d->send();
}

}
