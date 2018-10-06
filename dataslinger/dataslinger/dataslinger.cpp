#include "dataslinger.h"

#include <memory>
#include <vector>

#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/scope_exit.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/uuid/uuid.hpp>

#include <boost/process/async_pipe.hpp>
#include <boost/process/pipe.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

#include <chaiscript/chaiscript.hpp>

namespace dataslinger
{

class DataSlinger::DataSlingerImpl
{
public:
    DataSlingerImpl(DataSlinger* pQ) : q{pQ}
    {
    }

    ~DataSlingerImpl()
    {
    }

    void send()
    {
        BOOST_SCOPE_EXIT(&q) {
            q->signal_afterSend();
        } BOOST_SCOPE_EXIT_END

        q->signal_beforeSend();

        // TODO hook into engine
    }

private:
    // TODO send/receive data via named pipes or sockets
    // TODO chaiscript handlers on send/receipt of specific data types - should be fully programmable

    chaiscript::ChaiScript m_engine;

    DataSlinger* q;
};

DataSlinger::DataSlinger() : d{std::make_unique<DataSlinger::DataSlingerImpl>(this)}
{
}

DataSlinger::~DataSlinger()
{
}

void DataSlinger::send()
{
    d->send();
}

}
