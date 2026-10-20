#pragma once

#include "datastreamsourcefactory.h"
#include "iclock.h"
#include "ilogger.h"
#include "systemconfiguration.h"
#include "datastreamsourcestatistics.h"

#include <optional>
#include <memory>
#include <vector>

namespace qds
{

class DataStreamSourceManager
{
public:

  [[nodiscard]]
  bool initialize(
    const SystemConfiguration& configuration,
    const DataStreamSourceFactory& factory,
    IClock& clock,
    IDataBlockSink& blockSink,
    IDataStreamEventSink& eventSink,
    ILogger &logger);

  [[nodiscard]]
  bool start() noexcept;

  void stop() noexcept;

  [[nodiscard]]
  bool isRunning() const noexcept;

  [[nodiscard]]
  std::size_t size() const noexcept;

  [[nodiscard]]
  std::optional<DataStreamSourceStatistics> statistics(
    ModuleId id) const noexcept;

private:
  bool m_running = false;

  std::vector<std::unique_ptr<IDataStreamSource>> m_sources;
  std::vector<ModuleId> m_moduleIds;

  ILogger* m_logger = nullptr;
};

}