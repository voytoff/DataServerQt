#include "moduledatasource.h"

namespace qds
{

qds::ModuleDataSource::ModuleDataSource(
  IModuleDataSink &sink,
  const SystemConfiguration &cfg,
  ModuleId module,
  IModuleDevice &device,
  IClock &clock)
  : m_sink(sink)
  , m_cfg(cfg)
  , m_module(module)
  , m_device(device)
  , m_clock(clock)
{}

bool ModuleDataSource::start()
{
  if (m_running)
    return false;

  if (!m_device.start())
    return false;

  m_running = true;
  return true;
}

void ModuleDataSource::stop() noexcept
{
  if (!m_running)
    return;

  m_device.stop();

  m_running = false;
}

bool qds::ModuleDataSource::isRunning() const noexcept
{
  return m_running;
}

bool ModuleDataSource::step()
{
  if (!m_running)
    return false;

  auto timestamp = m_clock.now();

  const auto& tags = m_cfg.moduleTags(m_module);

  m_buffer.resize(tags.size());

  if (!m_device.read(m_buffer))
    return false;

  return m_sink.updateModule(
    m_module,
    m_buffer,
    timestamp);
}

}