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

  auto [it, inserted] =
    m_moduleTags.emplace(module.id, std::vector<TagId>{});

  assert(inserted);
}

bool SystemConfiguration::addTag(const TagInfo& tag)
{
  auto it = m_moduleTags.find(tag.module);

  if (it == m_moduleTags.end())
    return false;

  if (tag.tag.value >= m_tagExists.size())
  {
    m_tagExists.resize(tag.tag.value + 1, false);
    m_tagIndex.resize(tag.tag.value + 1, InvalidIndex32);
  }

  if (m_tagExists[tag.tag.value])
    return false;

  const uint32_t index =
    static_cast<uint32_t>(m_tags.size());

  m_tags.push_back(tag);

  m_tagExists[tag.tag.value] = true;
  m_tagIndex[tag.tag.value] = index;

  it->second.push_back(tag.tag);

  return true;
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
  auto it = m_moduleTags.find(id);

  assert(it != m_moduleTags.end());

  return it->second;
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

bool SystemConfiguration::addSignalDefinition(
  const SignalDefinition& definition)
{
  if (definition.kind == SignalKind::Raw && !containsTag(definition.source.tag))
    return false; // отсутствует источник

  auto id = definition.id.value;

  if (id >= m_signalDefinitionExists.size())
  {
    m_signalDefinitionExists.resize(id + 1, false);
    m_signalDefinitionIndex.resize(id + 1, InvalidIndex32);
  }

  if (m_signalDefinitionExists[id])
    return false; // дубликат

  assert(m_signalDefinitionIndex[id] == InvalidIndex32);

  const uint32_t index =
    static_cast<uint32_t>(m_signalDefinitions.size());

  m_signalDefinitions.push_back(definition);

  m_signalDefinitionExists[id] = true;
  m_signalDefinitionIndex[id] = index;

  return true;
}

const SignalDefinition* SystemConfiguration::findSignalDefinition(
  SignalId id) const
{
  if (id.value >= m_signalDefinitionIndex.size())
    return nullptr;

  const uint32_t index = m_signalDefinitionIndex[id.value];

  if (index == InvalidIndex32)
    return nullptr;

  assert(index < m_signalDefinitions.size());

  return &m_signalDefinitions[index];
}

uint32_t SystemConfiguration::moduleChannelCount(
  ModuleId id) const
{
  return static_cast<uint32_t>(
    moduleTags(id).size());
}

bool SystemConfiguration::containsSignalDefinition(
  SignalId id) const
{
  return (id.value < m_signalDefinitionExists.size()) &&
         m_signalDefinitionExists[id.value];
}

}