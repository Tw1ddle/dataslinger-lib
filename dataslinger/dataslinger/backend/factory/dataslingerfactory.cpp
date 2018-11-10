#include "dataslinger/backend/factory/dataslingerfactory.h"

#include <cassert>
#include <functional>
#include <optional>

#include "dataslinger/backend/pipe/datareceiverpipe.h"
#include "dataslinger/backend/sharedvector/sharedvector.h"
#include "dataslinger/backend/websocket/datareceiverwebsocket.h"
#include "dataslinger/backend/websocket/dataslingerwebsocket.h"
#include "dataslinger/connection/connectionoptions.h"
#include "dataslinger/event/event.h"
#include "dataslinger/message/message.h"

dataslinger::factory::Backend dataslinger::factory::makeSlingerBackend(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info)
{
    auto backend = info.getInfo().getValue<dataslinger::connection::PreferredBackend>(dataslinger::connection::ConnectionOption::PREFERRED_BACKEND);

    switch(backend) {
    case dataslinger::connection::PreferredBackend::WEBSOCKET_CLIENT:
        return makeBackendHelper<dataslinger::websocket::DataReceiverWebSocket>(onReceive, onEvent, info);
    case dataslinger::connection::PreferredBackend::WEBSOCKET_SERVER:
        return makeBackendHelper<dataslinger::websocket::DataSlingerWebSocket>(onReceive, onEvent, info);
    case dataslinger::connection::PreferredBackend::PIPE:
        return makeBackendHelper<dataslinger::pipe::DataReceiverPipe>(onReceive, onEvent, info);
    case dataslinger::connection::PreferredBackend::SHARED_VECTOR:
        return makeBackendHelper<dataslinger::sharedvector::SharedVector>(onReceive, onEvent, info);
    }

    assert(0 && "Bad options, unable to create backend");
    return makeBackendHelper<dataslinger::pipe::DataReceiverPipe>(onReceive, onEvent, info); // TODO
}
