#pragma once

#include "buffermanager.h"
#include "datasourcemanager.h"
#include "iarchivewriter.h"
#include "icalculationprocessor.h"
#include "iframepublisher.h"
#include "ischedulerclock.h"

namespace qds
{

class DataEngine
{
public:

  DataEngine() = default;

  bool initialize(
    DataSourceManager& manager,
    ICalculationProcessor& processor,
    BufferManager& buffers,
    IArchiveWriter& archive,
    IFramePublisher& publisher,
    ISchedulerClock &clock);

  bool process();

  void stop() noexcept;

  [[nodiscard]]
  bool isRunning() const noexcept;

private:

  DataSourceManager* m_manager = nullptr;
  ICalculationProcessor* m_processor = nullptr;

  BufferManager* m_buffers = nullptr;

  IArchiveWriter* m_archive = nullptr;
  IFramePublisher* m_publisher = nullptr;

  ISchedulerClock* m_clock;

  bool m_initialized = false;
};

}
