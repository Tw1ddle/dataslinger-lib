#include "dataslinger/slinger.h"

#include <cstdint>
#include <string>

namespace dataslinger::connection
{

const dataslinger::util::HeterogeneousMap<ConnectionOption>& ConnectionOptions::getInfo() const
{
    return m_info;
}

bool ConnectionOptions::hasWebSocketInfo() const
{
    return m_info.hasTypedValue<std::string>(ConnectionOption::WEBSOCKET_HOST_STRING)
            && m_info.hasTypedValue<std::uint16_t>(ConnectionOption::WEBSOCKET_PORT_UINT16);
}

}
