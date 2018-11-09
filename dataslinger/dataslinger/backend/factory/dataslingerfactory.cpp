#include "dataslinger/backend/factory/dataslingerfactory.h"

#include <functional>
#include <memory>

#include "dataslinger/backend/pipe/datareceiverpipe.h"
#include "dataslinger/backend/sharedvector/sharedvector.h"
#include "dataslinger/backend/websocket/datareceiverwebsocket.h"
#include "dataslinger/backend/websocket/dataslingerwebsocket.h"
#include "dataslinger/connection/connectioninfo.h"
#include "dataslinger/event/event.h"
#include "dataslinger/message/message.h"

dataslinger::factory::Backend dataslinger::factory::makeSlingerBackend(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionInfo& info)
{
    if(info.getInfo().hasValue(dataslinger::connection::ConnectionInfoDataKeys::WEBSOCKET_RECEIVER_HOST_STRING)) {
        return makeBackendHelper<dataslinger::websocket::DataReceiverWebSocket>(onReceive, onEvent, info);
    }
    return makeBackendHelper<dataslinger::websocket::DataSlingerWebSocket>(onReceive, onEvent, info);
}
