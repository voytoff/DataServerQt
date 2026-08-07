#pragma once

#include "buffermanager.h"
#include "iarchivewriter.h"
#include "icalculationprocessor.h"
#include "idatasource.h"
#include "iframepublisher.h"
#include "ischedulerclock.h"

namespace qds
{

class DataEngine
{
public:

  DataEngine() = default;

  bool initialize(
    IDataSource& source,
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

  IDataSource* m_source = nullptr;
  ICalculationProcessor* m_processor = nullptr;

  BufferManager* m_buffers = nullptr;

  IArchiveWriter* m_archive = nullptr;
  IFramePublisher* m_publisher = nullptr;

  ISchedulerClock* m_clock;

  bool m_initialized = false;
};

}
