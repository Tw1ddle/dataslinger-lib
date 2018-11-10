#pragma once

#include <functional>

#include "dataslinger/backend/dataslinger.h"
#include "dataslinger/connection/connectionoptions.h"
#include "dataslinger/event/event.h"
#include "dataslinger/message/message.h"

namespace dataslinger
{

DataSlinger makeDataSlinger(
        const std::function<void(const dataslinger::message::Message&)>& onReceive,
        const std::function<void(const dataslinger::event::Event&)>& onEvent,
        const dataslinger::connection::ConnectionOptions& info);

}
