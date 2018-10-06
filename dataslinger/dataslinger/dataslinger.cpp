#include "dataslinger.h"

#include <memory>
#include <vector>

#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>
#include <boost/poly_collection/base_collection.hpp>
#include <boost/poly_collection/function_collection.hpp>
#include <boost/process.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/uuid/uuid.hpp>

namespace dataslinger
{

class DataSlinger::DataSlingerImpl
{
public:
    DataSlingerImpl()
    {

    }

    ~DataSlingerImpl()
    {

    }
};

DataSlinger::DataSlinger() : d{std::make_unique<DataSlinger::DataSlingerImpl>()}
{
}

DataSlinger::~DataSlinger()
{
}

}
