#include "dataslinger/event/event.h"

#include <string>

namespace dataslinger
{

namespace event
{

std::string Event::what() const
{
    return m_info.getValueElse<std::string>(EventDataKeys::INFORMATIONAL_MESSAGE_STRING, std::string("Unknown event"));
}

const dataslinger::util::HeterogeneousMap<EventDataKeys>& Event::getInfo() const
{
    return m_info;
}

}

}
