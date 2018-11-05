#pragma once

#include <cstdint>

#include "dataslinger/util/heterogeneousmap.h"

namespace dataslinger
{

namespace connection
{

/// Enum whose values represent connection info field names/keys
enum class ConnectionInfoDataKeys
{
    WEBSOCKET_RECEIVER_HOST_STRING,
    WEBSOCKET_RECEIVER_PORT_UINT16,
    WEBSOCKET_SLINGER_HOST_STRING,
    WEBSOCKET_SLINGER_PORT_UINT16
};

/// Represents info used for establishing connections with the dataslinger/receivers
class ConnectionInfo
{
public:
    ConnectionInfo(const dataslinger::util::HeterogeneousMap<ConnectionInfoDataKeys>& info) : m_info{info} {}
    ~ConnectionInfo() = default;
    ConnectionInfo(const ConnectionInfo&) = default;
    ConnectionInfo& operator=(const ConnectionInfo&) = default;
    ConnectionInfo(ConnectionInfo&&) = default;
    ConnectionInfo& operator=(ConnectionInfo&&) = default;

    const dataslinger::util::HeterogeneousMap<ConnectionInfoDataKeys>& getInfo() const;

    /// Whether the given connection info contains required fields for creating a websocket-backed data slinger
    bool hasWebSocketSlingerInfo() const;
    /// Whether the given connection info contains required fields for creating a websocket-backed receiver
    bool hasWebSocketReceiverInfo() const;

private:
    dataslinger::util::HeterogeneousMap<ConnectionInfoDataKeys> m_info;
};

}

}
