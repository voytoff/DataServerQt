#include "datasourcemanager.h"

namespace qds
{

bool DataSourceManager::initialize(
  const SystemConfiguration &configuration,
  const SignalMemoryLayout &layout,
  const DataSourceFactory &factory)
{
  m_sources.clear();

  for (const auto &module : configuration.modules())
  {
    ModuleConfiguration mc;

    mc.module = module;
    mc.settings = module.settings;
    mc.channelCount =
      configuration.moduleChannelCount(module.id);

    auto source = factory.create(mc);

    if (source == nullptr)
    {
      m_sources.clear();
      return false;
    }

    const auto rawOffset = layout.rawOffset(module.id);

    if (!rawOffset)
    {
      m_sources.clear();
      return false;
    }

    m_sources.push_back(
      DataSourceEntry{
        .source = std::move(source),
        .rawOffset = rawOffset.value(),
        .channelCount = mc.channelCount
      });
  }

  return true;
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