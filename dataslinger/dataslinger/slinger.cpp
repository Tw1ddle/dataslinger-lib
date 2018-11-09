#pragma once

#include "slinger.h"

#include <functional>

#include "dataslinger/backend/dataslinger.h"
#include "dataslinger/connection/connectioninfo.h"
#include "dataslinger/event/event.h"
#include "dataslinger/message/message.h"

dataslinger::DataSlinger dataslinger::makeDataSlinger(
        const std::function<void(const dataslinger::message::Message&)>& onReceive,
        const std::function<void(const dataslinger::event::Event&)>& onEvent,
        const dataslinger::connection::ConnectionInfo& info)
{
    return dataslinger::DataSlinger(onReceive, onEvent, info);
}
