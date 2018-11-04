#pragma once

#include <cstdint>

#include "dataslinger/util/heterogeneousmap.h"

namespace dataslinger
{

namespace event
{

enum class EventDataKeys
{
    INFORMATIONAL_MESSAGE_STRING
};

/// Base class for errors and events emitted by the data slinger library
class Event
{
public:
    Event() = default;
    Event(const dataslinger::util::HeterogeneousMap<EventDataKeys>& info) : m_info{info} {}
    ~Event() = default;
    Event(const Event&) = default;
    Event& operator=(const Event&) = default;
    Event(Event&&) = default;
    Event& operator=(Event&&) = default;

    std::string what() const;
    const dataslinger::util::HeterogeneousMap<EventDataKeys>& getInfo() const;

private:
    dataslinger::util::HeterogeneousMap<EventDataKeys> m_info;
};

}

}
