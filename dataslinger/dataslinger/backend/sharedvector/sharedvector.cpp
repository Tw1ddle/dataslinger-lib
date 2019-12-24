#include "dataslinger/backend/sharedvector/sharedvector.h"

#include <functional>

#include <boost/scope_exit.hpp>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/segment_manager.hpp>

#include "dataslinger/slinger.h"
#include "dataslinger/event/eventhelpers.h"

namespace
{

class SharedData
{
public:
    SharedData(boost::interprocess::managed_shared_memory::segment_manager* segmentManager) :
        m_vector(segmentManager) {}

    boost::interprocess::vector<dataslinger::message::Message, boost::interprocess::allocator<dataslinger::message::Message, boost::interprocess::managed_shared_memory::segment_manager>> m_vector;
    boost::interprocess::interprocess_mutex mutex;
    boost::interprocess::interprocess_condition cond_empty;
};

}

namespace dataslinger::sharedvector
{

class SharedVector::SharedVectorImpl
{
public:
    SharedVectorImpl(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info) : m_sharedData{nullptr}, m_onReceive{onReceive}, m_onEvent{onEvent}, m_info{info}
    {
        // Create a new segment with given name and size
        boost::interprocess::managed_shared_memory segment(boost::interprocess::create_only, "MySharedMemory", 65536);

        // Initialize shared memory STL-compatible allocator
        //boost::interprocess::allocator<SharedData, boost::interprocess::managed_shared_memory::segment_manager> allocatorInstance();

        // Construct shared data in shared memory
        //m_sharedData = segment.construct<SharedData>("MySharedData")(allocatorInstance);
    }

    ~SharedVectorImpl()
    {
        boost::interprocess::shared_memory_object::remove("MySharedMemory");
    }

    SharedVectorImpl(const SharedVectorImpl&) = delete;
    SharedVectorImpl& operator=(const SharedVectorImpl&) = delete;
    SharedVectorImpl(SharedVectorImpl&&) = default;
    SharedVectorImpl& operator=(SharedVectorImpl&&) = default;

    void run()
    {
    }

    void send(const dataslinger::message::Message& /*message*/)
    {
    }

    void poll()
    {
    }

private:
    SharedData* m_sharedData;

    const std::function<void(const dataslinger::message::Message&)> m_onReceive; ///< Callback triggered when the slinger receives a message
    const std::function<void(const dataslinger::event::Event&)> m_onEvent; ///< Callback triggered when the slinger produces an event
    const dataslinger::connection::ConnectionOptions m_info; ///< Connection info
};

SharedVector::SharedVector(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info) : d{std::make_unique<SharedVectorImpl>(onReceive, onEvent, info)}
{
}

SharedVector::~SharedVector()
{
}

void SharedVector::run()
{
    d->run();
}

void SharedVector::send(const dataslinger::message::Message& message)
{
    d->send(message);
}

void SharedVector::poll()
{
    d->poll();
}

}
