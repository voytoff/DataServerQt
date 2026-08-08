#pragma once

#include "idatasource.h"
#include "signalmemorylayout.h"
#include "systemconfiguration.h"
#include <vector>

namespace qds
{

class DataSourceManager
{
public:
  bool initialize(
    const SystemConfiguration& configuration,
    const SignalMemoryLayout& layout);

  bool add(
    ModuleId module,
    std::unique_ptr<IDataSource> source);

  [[nodiscard]]
  bool acquire(RawMemory& memory);

  [[nodiscard]]
  std::size_t size() const noexcept;

private:
  struct Source
  {
    ModuleId module;
    uint32_t offset;
    std::unique_ptr<IDataSource> source;
  };

  std::vector<Source> m_sources;
};

}