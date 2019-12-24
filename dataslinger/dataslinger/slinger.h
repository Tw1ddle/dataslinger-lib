#pragma once

#include <any>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace dataslinger::util
{

/// Maps keys of one type to values of many (any) types in a map data structure
template<typename T> class HeterogeneousMap
{
public:
    HeterogeneousMap() = default;
    HeterogeneousMap(const std::map<T, std::any>& data) : m_map{data} {}
    ~HeterogeneousMap() = default;
    HeterogeneousMap(const HeterogeneousMap&) = default;
    HeterogeneousMap& operator=(const HeterogeneousMap&) = default;
    HeterogeneousMap(HeterogeneousMap&&) = default;
    HeterogeneousMap& operator=(HeterogeneousMap&&) = default;

    std::any& operator[](const T&& key)
    {
        return m_map[key];
    }

    bool hasValue(const T key) const
    {
        return m_map.find(key) != m_map.end();
    }

    template<typename V> bool hasTypedValue(const T key) const
    {
        if(!hasValue(key)) {
            return false;
        }

        try {
            std::any_cast<V>(m_map.at(key));
        } catch(const std::bad_any_cast&) {
            return false;
        }

        return true;
    }

    template<typename V> V getValue(const T key) const
    {
        return std::any_cast<V>(m_map.at(key));
    }

    template<typename V> V getValueElse(const T key, const V fallback) const
    {
        if(m_map.find(key) == m_map.end()) {
            return fallback;
        }

        try {
            return std::any_cast<V>(m_map.at(key));
        } catch(const std::bad_any_cast&) {
            return fallback;
        }

        return fallback;
    }

private:
    std::map<T, std::any> m_map;
};

}

namespace dataslinger::event
{

enum class EventKind
{
    INFORMATIONAL,
    ERROR
};

enum class EventSourceKind
{
    SLINGER,
    RECEIVER
};

enum class ErrorSeverity
{
    WARNING,
    FATAL
};

enum class EventDataKeys
{
    EVENT_KIND, ///< The sort of event e.g. informational, error
    EVENT_SOURCE_KIND, ///< The sort of source the event came from e.g. a slinger, a receiver
    ERROR_SEVERITY, ///< If the event is an error event, how serious is it
    MESSAGE_STRING, ///< Informational message describing what the event is
    INSTANCE_ID, ///< Event id
};

/// Base class for errors and events emitted by the data slinger library
class Event
{
public:
    Event() = default;
    Event(const dataslinger::util::HeterogeneousMap<EventDataKeys>& info);
    ~Event() = default;
    Event(const Event&) = default;
    Event& operator=(const Event&) = default;
    Event(Event&&) = default;
    Event& operator=(Event&&) = default;

    /// Returns an descriptive string describing what the event is for
    std::string what() const;

    /// Returns a numeric id for this event
    std::uint64_t instanceId() const;

    const dataslinger::util::HeterogeneousMap<EventDataKeys>& getInfo() const;

private:
    dataslinger::util::HeterogeneousMap<EventDataKeys> m_info;
};

}

namespace dataslinger::connection
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
    WEBSOCKET_PORT_UINT16,

    SHARED_VECTOR_INCOMING_SEGMENT_ID_STRING,
    SHARED_VECTOR_INCOMING_CAPACITY_BYTES_UINT64,
    SHARED_VECTOR_OUTGOING_SEGMENT_ID_STRING,
    SHARED_VECTOR_OUTGOING_CAPACITY_BYTES_UINT64
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

namespace dataslinger::message
{

using Message = std::vector<std::uint8_t>;

}

namespace dataslinger
{

/// The data slinger sends and receives data
class DataSlinger
{
public:
    DataSlinger(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info);
    ~DataSlinger();
    DataSlinger(const DataSlinger&) = delete;
    DataSlinger& operator=(const DataSlinger&) = delete;
    DataSlinger(DataSlinger&&);
    DataSlinger& operator=(DataSlinger&&);

    /// Sends a message to any connected/attached receivers (enqueues for sending on some backends)
    void send(const dataslinger::message::Message& message);

    /// Poll to process messages from any connected receivers and handle any events that have occurred
    void poll();

private:
    class DataSlingerImpl;
    std::unique_ptr<DataSlingerImpl> d;
};

DataSlinger makeDataSlinger(
        const std::function<void(const dataslinger::message::Message&)>& onReceive,
        const std::function<void(const dataslinger::event::Event&)>& onEvent,
        const dataslinger::connection::ConnectionOptions& info);

}
