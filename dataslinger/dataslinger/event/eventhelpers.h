#pragma once

#include "dataslinger/event/event.h"

#include <string>

namespace dataslinger
{

namespace event
{

/// Helper function that creates an informational event with source data
Event makeEvent(const dataslinger::event::EventSourceKind source, const std::string& message);

/// Helper function that creates an error event with source data
Event makeEvent(const dataslinger::event::EventSourceKind source, const dataslinger::event::ErrorSeverity errorSeverity, const std::string& message);

}

}
