#include "datareceiver.h"

#include <memory>
#include <vector>

#include <boost/scope_exit.hpp>

#include <chaiscript/chaiscript.hpp>

namespace dataslinger
{

class DataReceiver::DataReceiverImpl
{
public:
    DataReceiverImpl(DataReceiver* pQ) : q{pQ}
    {
    }

    ~DataReceiverImpl()
    {
    }

    void receive()
    {
        BOOST_SCOPE_EXIT(&q) {
            q->signal_afterReceive();
        } BOOST_SCOPE_EXIT_END

        q->signal_beforeReceive();

        // TODO hook into engine
    }

private:
    chaiscript::ChaiScript m_engine;

    DataReceiver* q;
};

DataReceiver::DataReceiver() : d{std::make_unique<DataReceiver::DataReceiverImpl>(this)}
{
}

DataReceiver::~DataReceiver()
{
}

}
