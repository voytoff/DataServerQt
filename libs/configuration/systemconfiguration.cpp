#include "systemconfiguration.h"
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
  if (tag.tag.value >= m_tagExists.size())
  {
    m_tagExists.resize(tag.tag.value + 1, false);
    m_tagIndex.resize(tag.tag.value + 1, InvalidIndex32);
  }

  assert(!m_tagExists[tag.tag.value]);
  assert(m_tagIndex[tag.tag.value] == InvalidIndex32);
  assert(tag.module.value < m_moduleTags.size());

  const uint32_t index =
    static_cast<uint32_t>(m_tags.size());

  m_tags.push_back(tag);

  m_tagExists[tag.tag.value] = true;
  m_tagIndex[tag.tag.value] = index;

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

const std::vector<SignalDefinition>& SystemConfiguration::signalDefinitions() const
{
  return m_signalDefinitions;
}

const std::vector<TagId>& SystemConfiguration::moduleTags(ModuleId id) const
{
  assert(id.value < m_moduleTags.size());

  return m_moduleTags[id.value];
}

bool SystemConfiguration::containsTag(TagId id) const
{
  return (id.value < m_tagExists.size()) &&
         m_tagExists[id.value];
}

const TagInfo* SystemConfiguration::findTag(TagId id) const
{  
  if (id.value >= m_tagIndex.size())
    return nullptr;

  const uint32_t index = m_tagIndex[id.value];

  if (index == InvalidIndex32)
    return nullptr;

  assert(index < m_tags.size());

  return &m_tags[index];
}

void SystemConfiguration::addSignalDefinition(
  const SignalDefinition& definition)
{
  if (definition.id >= m_signalDefinitionExists.size())
  {
    m_signalDefinitionExists.resize(definition.id + 1, false);
    m_signalDefinitionIndex.resize(definition.id + 1, InvalidIndex32);
  }

  assert(!m_signalDefinitionExists[definition.id]);
  assert(m_signalDefinitionIndex[definition.id] == InvalidIndex32);

  const uint32_t index =
    static_cast<uint32_t>(m_signalDefinitions.size());

  m_signalDefinitions.push_back(definition);

  m_signalDefinitionExists[definition.id] = true;
  m_signalDefinitionIndex[definition.id] = index;
}

const SignalDefinition* SystemConfiguration::findSignalDefinition(
  SignalId id) const
{
  if (id >= m_signalDefinitionIndex.size())
    return nullptr;

  const uint32_t index = m_signalDefinitionIndex[id];

  if (index == InvalidIndex32)
    return nullptr;

  assert(index < m_signalDefinitions.size());

  return &m_signalDefinitions[index];
}

bool SystemConfiguration::containsSignalDefinition(
  SignalId id) const
{
  return (id < m_signalDefinitionExists.size()) &&
         m_signalDefinitionExists[id];
}

}