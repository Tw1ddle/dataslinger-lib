#pragma once

#include <cstdint>

#include "dataslinger/util/heterogeneousmap.h"

namespace dataslinger
{

namespace connection
{

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
    bool hasWebSocketSlingerInfo() const;
    bool hasWebSocketReceiverInfo() const;

private:
    dataslinger::util::HeterogeneousMap<ConnectionInfoDataKeys> m_info;
};

}

}
