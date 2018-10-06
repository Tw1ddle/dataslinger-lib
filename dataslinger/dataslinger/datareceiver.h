#pragma once

#include <memory>

#include <boost/signals2/signal.hpp>

namespace dataslinger
{

/// Responsible for receiving data
class DataReceiver
{
public:
    DataReceiver();
    ~DataReceiver();
    DataReceiver(const DataReceiver&) = delete;
    DataReceiver& operator=(const DataReceiver&) = delete;

    boost::signals2::signal<void()> signal_beforeReceive;
    boost::signals2::signal<void()> signal_afterReceive;

private:
    class DataReceiverImpl;
    std::unique_ptr<DataReceiverImpl> d;
};

}
