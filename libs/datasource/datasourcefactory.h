#pragma once

#include <memory>

#include "idatasource.h"
#include "moduleinfo.h"

namespace qds
{

class DataSourceFactory
{
public:
  virtual ~DataSourceFactory() = default;

  [[nodiscard]]
  /*virtual */std::unique_ptr<IDataSource> create(
    const ModuleInfo& module); //const = 0;

};

}