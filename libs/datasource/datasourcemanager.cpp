#include "datasourcemanager.h"

namespace qds
{

bool DataSourceManager::initialize(
  const SystemConfiguration& configuration,
  const SignalMemoryLayout& layout,
  const DataSourceFactory& factory)
{
  m_sources.clear();

  for (const auto& module : configuration.modules())
  {
    const auto runtimeConfiguration =
      configuration.moduleRuntimeConfiguration(module.id);

    if (!runtimeConfiguration.has_value())
    {
      m_sources.clear();
      return false;
    }

    const uint32_t channelCount =
      static_cast<uint32_t>(runtimeConfiguration->tags.size());

    auto source =
      factory.create(runtimeConfiguration.value());

    if (source == nullptr)
    {
      m_sources.clear();
      return false;
    }

    const auto rawOffset =
      layout.rawOffset(module.id);

    if (!rawOffset)
    {
      m_sources.clear();
      return false;
    }

    m_sources.push_back(
      DataSourceEntry{
        .source = std::move(source),
        .rawOffset = rawOffset.value(),
        .channelCount = channelCount
      });
  }

  if (m_sources.empty())
    return false;

  return true;
}

bool DataSourceManager::start() noexcept
{
  for (auto& entry : m_sources)
  {
    if (!entry.source->start())
    {
      stop();
      return false;
    }
  }

  return true;
}

void DataSourceManager::stop() noexcept
{
  for (auto& entry : m_sources)
    entry.source->stop();
}

bool DataSourceManager::acquire(RawMemory& memory)
{
  for (const auto& entry : m_sources)
  {
    auto values =
      memory.values().subspan(
        entry.rawOffset,
        entry.channelCount);

    if (!entry.source->acquire(values))
      return false;
  }

  return true;
}

std::size_t DataSourceManager::size() const noexcept
{
  return m_sources.size();
}

}