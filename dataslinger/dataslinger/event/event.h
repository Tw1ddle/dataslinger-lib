#pragma once

#include <cstdint>

#include "dataslinger/util/heterogeneousmap.h"

namespace dataslinger
{

namespace event
{

enum class EventKind
{
    INFORMATIONAL,
    ERROR,
};

enum class EventSourceKind
{
    SLINGER,
    RECEIVER
};

enum class ErrorSeverity
{
    WARNING,
    FATAL
};

enum class EventDataKeys
{
    EVENT_KIND, ///< The sort of event e.g. informational, error
    EVENT_SOURCE_KIND, ///< The sort of source the event came from e.g. a slinger, a receiver
    ERROR_SEVERITY, ///< If the event is an error event, how serious is it
    MESSAGE_STRING, ///< Informational message describing what the event is
    INSTANCE_ID, ///< Event id
};

/// Base class for errors and events emitted by the data slinger library
class Event
{
public:
    Event() = default;
    Event(const dataslinger::util::HeterogeneousMap<EventDataKeys>& info);
    ~Event() = default;
    Event(const Event&) = default;
    Event& operator=(const Event&) = default;
    Event(Event&&) = default;
    Event& operator=(Event&&) = default;

    /// Returns an descriptive string describing what the event is for
    std::string what() const;

    /// Returns a numeric id for this event
    std::uint64_t instanceId() const;

    const dataslinger::util::HeterogeneousMap<EventDataKeys>& getInfo() const;

private:
    dataslinger::util::HeterogeneousMap<EventDataKeys> m_info;
};

}

}
