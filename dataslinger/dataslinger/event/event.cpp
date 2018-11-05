#include "dataslinger/event/event.h"

#include <atomic>
#include <cstdint>
#include <string>

namespace
{

std::atomic<std::uint64_t> id;

std::uint64_t createId()
{
    return id++;
}

}

namespace dataslinger
{

namespace event
{

Event::Event(const dataslinger::util::HeterogeneousMap<EventDataKeys>& info) : m_info{info}
{
    m_info[EventDataKeys::INSTANCE_ID] = createId();
}

std::string Event::what() const
{
    const std::string msg = m_info.getValueElse<std::string>(EventDataKeys::MESSAGE_STRING, std::string("Unknown event"));
    const std::string ret = std::to_string(instanceId()).append(": ").append(msg);
    return ret;
}

const dataslinger::util::HeterogeneousMap<EventDataKeys>& Event::getInfo() const
{
    return m_info;
}

std::uint64_t Event::instanceId() const
{
    return m_info.getValue<std::uint64_t>(EventDataKeys::INSTANCE_ID);
}

}

}
