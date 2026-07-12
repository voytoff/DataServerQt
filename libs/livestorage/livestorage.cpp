#include "livestorage.h"
#include "systemconfiguration.h"
#include <qassert.h>

namespace qds
{

LiveStorage::LiveStorage(const SystemConfiguration& cfg)
    : m_cfg(cfg)
{
    m_samples.resize(cfg.tags().size());
    m_moduleTimestamps.resize(cfg.modules().size(), 0);
}

const Sample& LiveStorage::sample(TagId id) const
{
  Q_ASSERT(id.value < m_samples.size());
  return m_samples[id.value];
}

uint64_t LiveStorage::moduleTimestamp(ModuleId id) const
{
  Q_ASSERT(id.value < m_moduleTimestamps.size());
  return m_moduleTimestamps[id.value];
}

size_t LiveStorage::size() const
{
return m_samples.size();
}

void LiveStorage::updateModule(ModuleId module,
                               std::span<const float> values,
                               uint64_t timestamp)
{
  const auto& tags = m_cfg.moduleTags(module);

  // 🔒 защита от некорректного входа
  Q_ASSERT(values.size() == tags.size());
  if (values.size() != tags.size())
  {
    // в реальном проекте можно логировать
    //m_invalidUpdates++;
    //m_lastError = Error::InvalidModuleSize;
    return;
  }

  // 🚀 быстрый прямой маппинг
  for (size_t i = 0; i < tags.size(); ++i)
  {
    const TagId tag = tags[i];
    auto& sample = m_samples[tag.value];
    sample.value = values[i];
    sample.quality = 0;
    //m_samples[tag.value].value = values[i];
    //m_samples[tag.value].quality = 0; // пока фиксируем
  }

  m_moduleTimestamps[module.value] = timestamp;
}

}
