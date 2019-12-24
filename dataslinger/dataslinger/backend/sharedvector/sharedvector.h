#pragma once

#include <functional>
#include <memory>

#include "dataslinger/slinger.h"

namespace dataslinger::sharedvector
{

/// Shared memory vector IPC backend
class SharedVector
{
public:
    SharedVector(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info);
    ~SharedVector();
    SharedVector(const SharedVector&) = delete;
    SharedVector& operator=(const SharedVector&) = delete;
    SharedVector(SharedVector&&) = default;
    SharedVector& operator=(SharedVector&&) = default;

    /// Call once to set the receiver up
    void run();

    /// Enqueue a message ready to be sent
    void send(const dataslinger::message::Message& message);

    /// Poll the receiver to process new messages received or events that have occurred
    void poll();

private:
    class SharedVectorImpl;
    std::unique_ptr<SharedVectorImpl> d;
};

}
