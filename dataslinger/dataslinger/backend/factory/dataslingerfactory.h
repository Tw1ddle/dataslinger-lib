#pragma once

#include <functional>
#include <memory>

#include "dataslinger/slinger.h"

namespace dataslinger::factory
{

struct Backend
{
    std::shared_ptr<void> m_impl;
    std::function<void()> run;
    std::function<void(const dataslinger::message::Message&)> send;
    std::function<void()> poll;
};

template<typename T> Backend makeBackendHelper(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info)
{
    std::shared_ptr<T> impl = std::make_shared<T>(onReceive, onEvent, info);
    T* pImpl = impl.get();

    Backend backend;
    backend.m_impl = impl;
    backend.run = [pImpl]() { pImpl->run(); };
    backend.send = [pImpl](const dataslinger::message::Message& msg) { pImpl->send(msg); };
    backend.poll = [pImpl]() { pImpl->poll(); };

    return backend;
}

Backend makeSlingerBackend(const std::function<void(const dataslinger::message::Message&)>& onReceive, const std::function<void(const dataslinger::event::Event&)>& onEvent, const dataslinger::connection::ConnectionOptions& info);

}
