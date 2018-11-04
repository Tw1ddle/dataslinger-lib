#pragma once

#include <cstdint>

namespace dataslinger
{

namespace message
{

namespace ids
{

enum MessageId : std::uint32_t
{
    namedInstruction = 0,
    scriptInvocation = 1,
    scriptInvocationWithOptions = 2
};

std::uint64_t computeMessageSizeBytes();

}

#pragma pack(push, 1)

/// Packed structure that represents the message/data sent from a data slinger to receiver(s)
struct Message
{
    dataslinger::message::ids::MessageId id;
};

#pragma pack(pop)

}

}
