#include "livestorage.h"
#include "systemconfiguration.h"
#include <cassert>

namespace qds
{

LiveStorage::LiveStorage(const SystemConfiguration& cfg)
    : m_cfg(cfg)
{
    m_samples.resize(cfg.tags().size());
    m_moduleTimestamps.resize(cfg.modules().size(), 0);
    m_tagModules.resize(cfg.tags().size());

    for (const auto& tag : cfg.tags())
      m_tagModules[tag.tag.value] = tag.module;
}

const Sample& LiveStorage::sample(TagId id) const noexcept
{
  assert(id.value < size());

  return m_samples[id.value];
}

bool LiveStorage::contains(TagId id) const noexcept
{
  return id.value < size();
}

uint64_t LiveStorage::moduleTimestamp(ModuleId id) const noexcept
{
  assert(id.value < m_moduleTimestamps.size());

  //if (id.value >= m_moduleTimestamps.size())
  //  return 0;

  return m_moduleTimestamps[id.value];
}

uint64_t LiveStorage::timestamp(TagId tag) const noexcept
{
  assert(tag.value < m_tagModules.size());

  //if (tag.value >= m_tagModules.size())
  //  return 0;

  return m_moduleTimestamps[m_tagModules[tag.value].value];
}

std::size_t LiveStorage::size() const noexcept
{
  return m_samples.size();
}

bool LiveStorage::updateModule(ModuleId module, std::span<const float> values, uint64_t timestamp)
{
  const auto& tags = m_cfg.moduleTags(module);

  // 🔒 защита от некорректного входа
  //Q_ASSERT(values.size() == tags.size());
  if (values.size() != tags.size())
  {
    /* TODO перенести наверх
    qWarning()
      << "LiveStorage::updateModule(): ожидалось"
      << tags.size()
      << "значений, получено"
      << values.size();
    */
    return false;
  }

  for (size_t i = 0; i < tags.size(); ++i)
  {
    const TagId tag = tags[i];
    auto& sample = m_samples[tag.value];
    sample.value = values[i];
  }

  m_moduleTimestamps[module.value] = timestamp;
  return true;
}

}
