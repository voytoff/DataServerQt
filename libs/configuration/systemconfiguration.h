#pragma once

#include <string_view>
#include <strongidhash.h>
#include <vector>
#include "taginfo.h"
#include "moduleinfo.h"
#include "crateinfo.h"
#include "datatypes.h"
#include "signaldefinition.h"

namespace qds
{

/*
crate
 ├── id
 ├── type
 ├── serial
 ├── host
 ├── port
 └── description

module
 ├── id
 ├── crate_id
 ├── type
 ├── serial
 └── description

configuration
 └── ...

configuration_module
 ├── configuration_id
 ├── module_id
 └── settings JSON

configuration_tag
 ├── configuration_id
 ├── id
 ├── module_id
 └── channel

configuration_signal_definition
 ├── configuration_id
 ├── id
 ├── ...
 └── dependencies
*/

class SystemConfiguration
{
public:
  void addCrate(const CrateInfo& crate);
  void addModule(const ModuleInfo& module);
  bool addTag(const TagInfo& tag);
  bool addSignalDefinition(const SignalDefinition& definition);

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
  const SignalDefinition* findSignalDefinition(std::string_view name) const;

  [[nodiscard]]
  uint32_t moduleChannelCount(ModuleId id) const;

private:

  std::vector<CrateInfo> m_crates;

  std::vector<ModuleInfo> m_modules;
  std::unordered_map<ModuleId, std::vector<TagId>> m_moduleTags;

  std::vector<TagInfo> m_tags;
  // быстрый поиск TagId -> индекс в m_tags
  std::vector<uint32_t> m_tagIndex;
  std::vector<bool> m_tagExists;

  std::vector<SignalDefinition> m_signalDefinitions;
  // быстрый поиск SignalId -> индекс в m_signals
  std::vector<uint32_t> m_signalDefinitionIndex;
  std::vector<bool> m_signalDefinitionExists;

};

}