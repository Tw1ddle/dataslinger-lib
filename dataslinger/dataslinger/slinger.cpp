#include "slinger.h"

#include <functional>

#include "dataslinger/slinger.h"

dataslinger::DataSlinger dataslinger::makeDataSlinger(
        const std::function<void(const dataslinger::message::Message&)>& onReceive,
        const std::function<void(const dataslinger::event::Event&)>& onEvent,
        const dataslinger::connection::ConnectionOptions& info)
{
    return dataslinger::DataSlinger(onReceive, onEvent, info);
}
