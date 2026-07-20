#include "livestorage.h"
#include "systemconfiguration.h"
#include <qassert.h>
#include <qdebug.h>
#include <qlogging.h>

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
  Q_ASSERT(id.value < size());
  return m_samples[id.value];
}

bool LiveStorage::read(TagId tag, Sample& sample) const
{
  if (tag.value >= size())
    return false;

  sample = m_samples[tag.value];
  return true;
}

uint64_t LiveStorage::moduleTimestamp(ModuleId id) const noexcept
{
  Q_ASSERT(id.value < m_moduleTimestamps.size());
  return m_moduleTimestamps[id.value];
}

std::size_t LiveStorage::size() const noexcept
{
  return m_samples.size();
}

bool LiveStorage::updateModule(ModuleId module,
                               std::span<const float> values,
                               uint64_t timestamp)
{
  const auto& tags = m_cfg.moduleTags(module);

  // 🔒 защита от некорректного входа
  //Q_ASSERT(values.size() == tags.size());
  if (values.size() != tags.size())
  {
    qWarning()
      << "LiveStorage::updateModule(): ожидалось"
      << tags.size()
      << "значений, получено"
      << values.size();
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
