#pragma once

#include <cstdint>

#include "dataslinger/heterogeneousmap.h"

namespace dataslinger
{

namespace error
{

enum ErrorDataKeys : std::int32_t
{
    MESSAGE_STRING = 0
};

/// Base class for exceptions emitted by the data slinger library
class DataSlingerError
{
public:
    DataSlingerError(const dataslinger::HeterogeneousMap<std::int32_t>& info) : m_info{info}
    {
    }
    ~DataSlingerError() = default;

    std::string what() const
    {
        return m_info.getValueElse<std::string>(MESSAGE_STRING, std::string("Unknown error"));
    }

    const dataslinger::HeterogeneousMap<std::int32_t>& getInfo() const
    {
        return m_info;
    }

private:
    dataslinger::HeterogeneousMap<std::int32_t> m_info;
};

}

}
