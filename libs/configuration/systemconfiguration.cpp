#include "systemconfiguration.h"

namespace qds
{

void SystemConfiguration::addCrate(const CrateInfo& crate)
{
  m_crates.push_back(crate);
}

void SystemConfiguration::addModule(const ModuleInfo& module)
{
  m_modules.push_back(module);
  m_moduleTags.emplace_back();
}

void SystemConfiguration::addTag(const TagInfo& tag)
{
  m_tags.push_back(tag);

  TagRecord rec;
  //?rec.module = tag.module;
  //?rec.channel = tag.channel;

  m_tagIndex.push_back(rec);

  // важно: индекс по module.value
  m_moduleTags[tag.module.value].push_back(tag.tag);
}

const std::vector<CrateInfo>& SystemConfiguration::crates() const
{
  return m_crates;
}

const std::vector<ModuleInfo>& SystemConfiguration::modules() const
{
  return m_modules;
}

const std::vector<TagInfo>& SystemConfiguration::tags() const
{
  return m_tags;
}

const TagRecord SystemConfiguration::tagRecord(TagId id) const
{
  return m_tagIndex[id.value];
}

const std::vector<TagId>& SystemConfiguration::moduleTags(ModuleId id) const
{
  return m_moduleTags[id.value];
}

}