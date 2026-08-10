#pragma once

#include <vector>
#include "taginfo.h"
#include "moduleinfo.h"
#include "crateinfo.h"
#include "datatypes.h"
#include "signaldefinition.h"

namespace qds
{

class SystemConfiguration
{
public:
  void addCrate(const CrateInfo& crate);
  void addModule(const ModuleInfo& module);
  void addTag(const TagInfo& tag);
  void addSignalDefinition(const SignalDefinition& definition);

  const std::vector<CrateInfo>& crates() const;
  const std::vector<ModuleInfo>& modules() const;
  const std::vector<TagInfo>& tags() const;
  const std::vector<SignalDefinition>& signalDefinitions() const;

  const std::vector<TagId>& moduleTags(ModuleId id) const;

  [[nodiscard]]
  bool containsTag(TagId id) const;
  [[nodiscard]]
  const TagInfo* findTag(TagId id) const;

  [[nodiscard]]
  bool containsSignalDefinition(SignalId id) const;
  [[nodiscard]]
  const SignalDefinition* findSignalDefinition(SignalId id) const;

  [[nodiscard]]
  uint32_t moduleChannelCount(ModuleId id) const;

private:

  std::vector<CrateInfo> m_crates;
  std::vector<ModuleInfo> m_modules;
  std::vector<TagInfo> m_tags;

  std::vector<SignalDefinition> m_signalDefinitions;

  std::vector<std::vector<TagId>> m_moduleTags;

  // быстрый поиск TagId -> индекс в m_tags
  std::vector<uint32_t> m_tagIndex;
  std::vector<bool> m_tagExists;

  // быстрый поиск SignalId -> индекс в m_signals
  std::vector<uint32_t> m_signalDefinitionIndex;
  std::vector<bool> m_signalDefinitionExists;

};

}