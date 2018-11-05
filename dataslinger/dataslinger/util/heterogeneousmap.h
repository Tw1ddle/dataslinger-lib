#pragma once

#include <map>

#include <boost/any.hpp>

namespace dataslinger
{

namespace util
{

/// Maps keys of one type to values of many (any) types in a map data structure
template<typename T> class HeterogeneousMap
{
public:
    HeterogeneousMap() = default;
    HeterogeneousMap(const std::map<T, boost::any>& data) : m_map{data} {}
    ~HeterogeneousMap() = default;
    HeterogeneousMap(const HeterogeneousMap&) = default;
    HeterogeneousMap& operator=(const HeterogeneousMap&) = default;
    HeterogeneousMap(HeterogeneousMap&&) = default;
    HeterogeneousMap& operator=(HeterogeneousMap&&) = default;

    boost::any& operator[](const T&& key)
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
            boost::any_cast<V>(m_map.at(key));
        } catch(const boost::bad_any_cast&) {
            return false;
        }

        return true;
    }

    template<typename V> V getValue(const T key) const
    {
        return boost::any_cast<V>(m_map.at(key));
    }

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

}
