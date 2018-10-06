#pragma once

#include <cstdint>

namespace dataslinger
{

namespace message
{

using MessageId = std::uint32_t;

namespace ids
{

MessageId namedInstruction = 0;
MessageId scriptInvocation = 1;
MessageId scriptInvocationWithOptions = 2;

std::uint64_t computeMessageSizeBytes()
{
    return 0; // TODO compute size?
}

}

#pragma pack(push, 1)

/// Packed structure that represents the message/data sent from a data slinger to receiver(s)
struct Message
{
    ids::MessageId id;
};

#pragma pack pop()

}

}
