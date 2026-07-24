#include "moduledatasource.h"
#include <qdebug.h>
#include <qlogging.h>

namespace qds
{

ModuleDataSource::ModuleDataSource(
  IModuleDataSink &sink,
  const SystemConfiguration &cfg,
  const ModuleInfo& module,
  std::unique_ptr<qds::IHardwareModule> device,
  IClock &clock)
  : m_sink(sink)
  , m_cfg(cfg)
  , m_module(module)
  , m_device(std::move(device))
  , m_clock(clock) { }

bool ModuleDataSource::start()
{
  if (m_running)
    return false;

  if (!m_device->start())
    return false;

  m_running = true;
  return true;
}

void ModuleDataSource::stop() noexcept
{
  if (!m_running)
    return;

  m_device->stop();

  m_running = false;
}

bool ModuleDataSource::isRunning() const noexcept
{
  return m_running;
}

bool ModuleDataSource::step()
{
  if (!m_running)
    return false;

  auto timestamp = m_clock.now();

  const auto& tags = m_cfg.moduleTags(m_module.id);

  m_buffer.resize(tags.size());

  if (!m_device->read(m_buffer))
    return false;

  return m_sink.updateModule(
    m_module.id,
    m_buffer,
    timestamp);
}

}