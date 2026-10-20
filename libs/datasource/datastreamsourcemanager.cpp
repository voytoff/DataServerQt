#include "datastreamsourcemanager.h"

namespace qds
{

bool DataStreamSourceManager::initialize(
  const SystemConfiguration& configuration,
  const DataStreamSourceFactory& factory,
  IClock& clock,
  IDataBlockSink& blockSink,
  IDataStreamEventSink& eventSink,
  ILogger& logger)
{
  if (m_running)
  {
    logger.error(
      "DataStreamSourceManager: already running");
    return false;
  }

  std::vector<std::unique_ptr<IDataStreamSource>> sources;

  for (const auto& module : configuration.modules())
  {
    const auto type = module.type;

    if (type == ModuleType::Unknown)
    {
      logger.error(
        "DataStreamSourceManager: unknown module type, id=" +
        std::to_string(module.id.value));

      return false;
    }

    const auto cfg =
      configuration.moduleRuntimeConfiguration(module.id);

    if (!cfg.has_value())
    {
      logger.error(
        "DataStreamSourceManager: missing runtime configuration, module id=" +
        std::to_string(module.id.value));

      return false;
    }

    auto source =
      factory.create(
        cfg.value(),
        clock,
        blockSink,
        eventSink);

    if (!source)
    {
      logger.error(
        "DataStreamSourceManager: failed to create source, module id=" +
        std::to_string(module.id.value) +
        ", type=" +
        std::to_string(static_cast<int>(type)));

      return false;
    }

    sources.push_back(std::move(source));
  }

  m_sources = std::move(sources);
  m_logger = &logger;

  return true;
}

bool DataStreamSourceManager::start() noexcept
{
  if (isRunning())
    return true;

  for (std::size_t i = 0; i < m_sources.size(); ++i)
  {
    if (!m_sources[i]->start())
    {
      if (m_logger)
      {
        m_logger->error(
          "Не удалось запустить источник данных, индекс=" +
          std::to_string(i));
      }

      for (auto& source : m_sources)
        source->stop();

      m_running = false;
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
  if (!m_running)
    return false;

  for (const auto& source : m_sources)
  {
    if (!source->isRunning())
      return false;
  }

  return true;
}

std::size_t DataStreamSourceManager::size() const noexcept
{
  return m_sources.size();
}

}