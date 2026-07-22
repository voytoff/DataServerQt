#ifndef DATAENGINE_H
#define DATAENGINE_H

#include "datasourcemanager.h"
#include "livescheduler.h"

namespace qds
{

class DataEngine
{
public:
  DataEngine(
    DataSourceManager& sources,
    LiveScheduler& scheduler);

  [[nodiscard]] bool start() noexcept;
  void stop();

  [[nodiscard]] bool isRunning() const noexcept;

  [[nodiscard]] bool step();

private:
  bool m_running = false;

  DataSourceManager& m_sources;
  LiveScheduler& m_scheduler;
};

}

#endif // DATAENGINE_H
