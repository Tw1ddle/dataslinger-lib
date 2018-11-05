#pragma once

#include <cstdint>
#include <vector>

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

/// Packed structure that represents the message/data sent between data slingers and receivers
struct Message
{
    dataslinger::message::ids::MessageId id;
    std::vector<std::byte> m_data;
};

#pragma pack(pop)

}

}
