#include "periodicdatasourcerunner.h"

namespace qds
{

PeriodicDataSourceRunner::PeriodicDataSourceRunner(
  GeneratorDataSource& source,
  QObject* parent)
  : QObject(parent)
  , m_source(source)
{
  connect(&m_timer, &QTimer::timeout, this, [this]
    {
      m_source.step();
    });
}

void PeriodicDataSourceRunner::start(int intervalMs)
{
  if (m_timer.isActive())
    return;

  if (!m_source.start())
    return;

  m_timer.start(intervalMs);
}

void PeriodicDataSourceRunner::stop()
{
  m_timer.stop();
  m_source.stop();
}

bool PeriodicDataSourceRunner::isRunning() const
{
  return m_timer.isActive();
}

}