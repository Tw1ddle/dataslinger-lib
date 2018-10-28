#pragma once

#include <memory>

#include <boost/signals2/signal.hpp>

namespace dataslinger
{

/// Responsible for sending data
class DataSlinger
{
public:
    DataSlinger();
    ~DataSlinger();
    DataSlinger(const DataSlinger&) = delete;
    DataSlinger& operator=(const DataSlinger&) = delete;
    DataSlinger(DataSlinger&&);
    DataSlinger& operator=(DataSlinger&&);

    boost::signals2::signal<void()> signal_beforeSend;
    boost::signals2::signal<void()> signal_afterSend;
    void send();

private:
    class DataSlingerImpl;
    std::unique_ptr<DataSlingerImpl> d;
};

}
