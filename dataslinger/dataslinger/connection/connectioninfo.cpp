#include "dataslinger/connection/connectioninfo.h"

#include <cstdint>

namespace dataslinger
{

namespace connection
{

const dataslinger::util::HeterogeneousMap<ConnectionInfoDataKeys>& ConnectionInfo::getInfo() const
{
    return m_info;
}

bool ConnectionInfo::hasWebSocketSlingerInfo() const
{
    return m_info.hasTypedValue<std::string>(ConnectionInfoDataKeys::WEBSOCKET_SLINGER_HOST_STRING)
            && m_info.hasTypedValue<std::uint16_t>(ConnectionInfoDataKeys::WEBSOCKET_SLINGER_PORT_UINT16);
}

bool ConnectionInfo::hasWebSocketReceiverInfo() const
{
    return m_info.hasTypedValue<std::string>(ConnectionInfoDataKeys::WEBSOCKET_RECEIVER_HOST_STRING)
            && m_info.hasTypedValue<std::uint16_t>(ConnectionInfoDataKeys::WEBSOCKET_RECEIVER_PORT_UINT16);
}

}

}
