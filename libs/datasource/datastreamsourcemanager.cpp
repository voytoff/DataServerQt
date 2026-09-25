#include "datastreamsourcemanager.h"

namespace qds
{

bool DataStreamSourceManager::initialize(
  const SystemConfiguration &configuration,
  const DataStreamSourceFactory &factory,
  IClock &clock, IDataBlockSink &blockSink,
  IDataStreamEventSink &eventSink)
{
  if (m_running)
    return false;

  std::vector<std::unique_ptr<IDataStreamSource>> sources;
  for (const auto &module : configuration.modules())
  {
    const auto type = module.type;

    if (type == ModuleType::Unknown)
      return false;

    const auto cfg = configuration.moduleRuntimeConfiguration(module.id);

    if (!cfg.has_value())
      return false;

    auto source =
      factory.create(
        cfg.value(),
        clock,
        blockSink,
        eventSink);

    if (!source)
      return false;

    sources.push_back(std::move(source));
  }

  m_sources = std::move(sources);
  return true;
}

bool DataStreamSourceManager::start() noexcept
{
  if (m_running)
    return true;

  std::size_t started = 0;

  for (; started < m_sources.size(); ++started)
  {
    if (!m_sources[started]->start())
    {
      while (started > 0)
      {
        --started;
        m_sources[started]->stop();
      }

      return false;
    }
  }

  m_running = true;
  return true;
}

void DataStreamSourceManager::stop() noexcept
{
  if (!m_running)
    return;

  for (auto& source : m_sources)
    source->stop();

  m_running = false;
}

bool DataStreamSourceManager::isRunning() const noexcept
{
  return m_running;
}

std::size_t DataStreamSourceManager::size() const noexcept
{
  return m_sources.size();
}

}
