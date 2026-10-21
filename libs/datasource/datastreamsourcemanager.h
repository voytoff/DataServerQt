#pragma once

#include "datastreamsourcefactory.h"
#include "datastreamsourcestate.h"
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
  IDataStreamSource* find(
    ModuleId id) const noexcept;

  /*
   * true         — источник работает;
   * false        — источник остановлен;
   * std::nullopt — источник с таким идентификатором не найден.
   */
  [[nodiscard]]
  std::optional<bool> isRunning(
    ModuleId id) const noexcept;

  [[nodiscard]]
  std::size_t size() const noexcept;

  [[nodiscard]]
  std::optional<DataStreamSourceStatistics> statistics(
    ModuleId id) const noexcept;

  [[nodiscard]]
  std::optional<DataStreamSourceState> state(
    ModuleId id) const noexcept;

private:
  bool m_running = false;

  std::vector<std::unique_ptr<IDataStreamSource>> m_sources;
  std::vector<ModuleId> m_moduleIds;

  ILogger* m_logger = nullptr;
};

}