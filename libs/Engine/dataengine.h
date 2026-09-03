#pragma once

#include "buffermanager.h"
#include "datasourcemanager.h"
#include "iframepublisher.h"
#include "iarchivewriter.h"
#include "ilogger.h"
#include "ischedulerclock.h"
#include "signalprocessor.h"

namespace qds
{

class DataEngine
{
public:

  DataEngine() = default;

  [[nodiscard]]
  bool initialize(
    DataSourceManager& dataSources,
    SignalProcessor& signalProcessor,
    BufferManager& buffers,
    IArchiveWriter& archive,
    IFramePublisher& publisher,
    ISchedulerClock& clock,
    ILogger& logger) noexcept;

  [[nodiscard]]
  bool process() noexcept;

  void stop() noexcept;

  [[nodiscard]]
  bool isRunning() const noexcept;

private:

  DataSourceManager* m_dataSources = nullptr;
  SignalProcessor* m_signalProcessor = nullptr;

  BufferManager* m_buffers = nullptr;

  IArchiveWriter* m_archive = nullptr;
  IFramePublisher* m_publisher = nullptr;

  ISchedulerClock* m_clock = nullptr;

  ILogger* m_logger = nullptr;

  bool m_initialized = false;
  bool m_running = false;
};

}