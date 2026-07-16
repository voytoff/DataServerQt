#include "systemconfiguration.h"
#include <algorithm>
#include <cassert>

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
  assert(tag.module.value < m_moduleTags.size());

  if (tag.tag.value >= m_tagExists.size())
    m_tagExists.resize(tag.tag.value + 1, false);

  assert(!m_tagExists[tag.tag.value]);

  m_tagExists[tag.tag.value] = true;

  m_tags.push_back(tag);
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

const std::vector<TagId>& SystemConfiguration::moduleTags(ModuleId id) const
{
  assert(id.value < m_moduleTags.size());
  return m_moduleTags[id.value];
}

bool SystemConfiguration::containsTag(TagId id) const
{
  return std::any_of(
    m_tags.begin(),
    m_tags.end(),
    [id](const TagInfo& t)
    {
      return t.tag == id;
    });
}

const TagInfo *SystemConfiguration::findTag(TagId id) const
{
  for (const auto& tag : m_tags)
  {
    if (tag.tag == id)
      return &tag;
  }

  return nullptr;
}

}