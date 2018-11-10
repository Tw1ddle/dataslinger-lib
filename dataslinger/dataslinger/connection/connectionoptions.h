#pragma once

#include "dataslinger/util/heterogeneousmap.h"

namespace dataslinger
{

namespace connection
{

enum class PreferredBackend
{
    PIPE,
    SHARED_VECTOR,
    WEBSOCKET_CLIENT,
    WEBSOCKET_SERVER
};

/// Enum whose values represent connection options field names/keys
enum class ConnectionOption
{
    PREFERRED_BACKEND,

    WEBSOCKET_HOST_STRING,
    WEBSOCKET_PORT_UINT16
};

/// Represents info used for establishing connections dataslingers/receivers
class ConnectionOptions
{
public:
    ConnectionOptions(const dataslinger::util::HeterogeneousMap<ConnectionOption>& info) : m_info{info} {}
    ~ConnectionOptions() = default;
    ConnectionOptions(const ConnectionOptions&) = default;
    ConnectionOptions& operator=(const ConnectionOptions&) = default;
    ConnectionOptions(ConnectionOptions&&) = default;
    ConnectionOptions& operator=(ConnectionOptions&&) = default;

    const dataslinger::util::HeterogeneousMap<ConnectionOption>& getInfo() const;

    /// Whether the given connection info contains required fields for creating a websocket-backed data slinger
    bool hasWebSocketInfo() const;

private:
    dataslinger::util::HeterogeneousMap<ConnectionOption> m_info;
};

}

}
