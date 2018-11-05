#pragma once

#include <cstdint>

#include "dataslinger/util/heterogeneousmap.h"

namespace dataslinger
{

namespace event
{

enum class EventSourceKind
{
    SLINGER,
    RECEIVER
};

enum class EventKind
{
    INFORMATIONAL,
    ERROR,
};

enum class ErrorSeverity
{
    WARNING,
    FATAL
};

enum class EventDataKeys
{
    EVENT_KIND,
    EVENT_SOURCE_KIND,
    INSTANCE_ID,
    ERROR_SEVERITY,
    MESSAGE_STRING,
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

    /// Returns an integer id (atomic count starting at 0) for this event
    std::uint64_t instanceId() const;

    const dataslinger::util::HeterogeneousMap<EventDataKeys>& getInfo() const;

private:
    dataslinger::util::HeterogeneousMap<EventDataKeys> m_info;
};

}

}
