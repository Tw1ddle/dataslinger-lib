#include "dataslinger/event/eventhelpers.h"

#include <string>

namespace dataslinger
{

namespace event
{

Event makeEvent(const dataslinger::event::EventSourceKind source, const std::string& message)
{
    return dataslinger::event::Event({{{
        { dataslinger::event::EventDataKeys::EVENT_KIND, dataslinger::event::EventKind::INFORMATIONAL },
        { dataslinger::event::EventDataKeys::EVENT_SOURCE_KIND, source },
        { dataslinger::event::EventDataKeys::MESSAGE_STRING, std::string(message) }
    }}});
}

Event makeEvent(const dataslinger::event::EventSourceKind source, const dataslinger::event::ErrorSeverity errorSeverity, const std::string& message)
{
    return dataslinger::event::Event({{{
        { dataslinger::event::EventDataKeys::EVENT_KIND, dataslinger::event::EventKind::ERROR },
        { dataslinger::event::EventDataKeys::EVENT_SOURCE_KIND, source },
        { dataslinger::event::EventDataKeys::ERROR_SEVERITY, errorSeverity },
        { dataslinger::event::EventDataKeys::MESSAGE_STRING, std::string(message) }
    }}});
}

}

}
