#pragma once

#include <map>

#include <boost/any.hpp>

namespace dataslinger
{

/// Maps keys of one type to values of many (any) types in a map data structure
template<typename T> class HeterogeneousMap
{
public:
    HeterogeneousMap(const std::map<T, boost::any>& data) : m_map{data} {}
    ~HeterogeneousMap() = default;
    HeterogeneousMap(const HeterogeneousMap&) = default;
    HeterogeneousMap& operator=(const HeterogeneousMap&) = default;
    HeterogeneousMap(HeterogeneousMap&&) = default;
    HeterogeneousMap& operator=(HeterogeneousMap&&) = default;

    template<typename V> V getValueElse(const T key, const V fallback) const
    {
        if(m_map.find(key) == m_map.end()) {
            return fallback;
        }

        try {
            return boost::any_cast<V>(m_map.at(key));
        } catch(const boost::bad_any_cast&) {
            return fallback;
        }

        return fallback;
    }

private:
    std::map<T, boost::any> m_map;
};

}
