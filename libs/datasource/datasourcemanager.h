#pragma once

#include <vector>

#include "datasourcefactory.h"
#include "signalmemory.h"
#include "signalmemorylayout.h"
#include "systemconfiguration.h"

namespace qds
{

struct DataSourceEntry
{
  std::unique_ptr<IDataSource> source;

  uint32_t rawOffset = 0;
  uint32_t channelCount = 0;
};

class DataSourceManager
{
public:
  bool initialize(
    const SystemConfiguration& configuration,
    const SignalMemoryLayout& layout,
    const DataSourceFactory& factory);

  [[nodiscard]]
  bool acquire(RawMemory& memory);

  [[nodiscard]]
  std::size_t size() const noexcept;

private:
  std::vector<DataSourceEntry> m_sources;

};

}