#pragma once

#include <string_view>
#include <strongidhash.h>
#include <vector>
#include "configurationmodule.h"
#include "configurationtag.h"
#include "moduleruntimeconfiguration.h"
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
 ├── udpPort
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
 ├── formula_expression
 ├── ...
 └── dependencies
*/

class SystemConfiguration
{
public:
  void addCrate(const CrateInfo& crate);

  bool addModule(const ModuleInfo& module);
  bool addTag(const TagInfo& tag);
  bool addSignalDefinition(const SignalDefinition& definition);

  const std::vector<CrateInfo>& crates() const noexcept;
  const std::vector<ModuleInfo>& modules() const noexcept;
  const std::vector<TagInfo>& tags() const noexcept;
  const std::vector<SignalDefinition>& signalDefinitions() const noexcept;

  const std::vector<TagId>& moduleTags(ModuleId id) const;

  bool containsTag(TagId id) const;
  const TagInfo* findTag(TagId id) const;

  SignalDefinition* findSignalDefinition(SignalId id);
  const SignalDefinition* findSignalDefinition(SignalId id) const;

  bool containsSignalDefinition(SignalId id) const;

  const SignalDefinition* findSignalDefinition(
    std::string_view name) const;

  uint32_t moduleChannelCount(ModuleId id) const;

  const ModuleInfo* findModule(ModuleId id) const;
  const CrateInfo* findCrate(CrateId id) const;

  bool addConfigurationModule(
    const ConfigurationModule& configuration);

  bool addConfigurationTag(
    const ConfigurationTag& configuration);

  const ConfigurationModule* findConfigurationModule(
    ModuleId id) const;

  std::optional<ModuleRuntimeConfiguration> moduleRuntimeConfiguration(
    ModuleId id) const;

  void setUdpPort(uint16_t port);
  uint16_t udpPort() const noexcept;

  void setName(std::string name);
  const std::string& name() const noexcept;

  void setDescription(std::string description);
  const std::string& description() const noexcept;

private:
  std::vector<CrateInfo> m_crates;
  std::vector<ModuleInfo> m_modules;

  std::vector<TagInfo> m_tags;
  std::vector<uint32_t> m_tagIndex;
  std::vector<bool> m_tagExists;

  std::unordered_map<ModuleId, std::vector<TagId>> m_moduleTags;

  std::vector<SignalDefinition> m_signalDefinitions;
  std::vector<uint32_t> m_signalDefinitionIndex;
  std::vector<bool> m_signalDefinitionExists;

  std::vector<ConfigurationModule> m_configurationModules;

  std::vector<ConfigurationTag> m_configurationTags;

  // ModuleId -> indices in m_configurationTags
  std::unordered_map<ModuleId, std::vector<uint32_t>>
    m_configurationModuleTags;

  uint16_t m_udpPort = 0;

  std::string m_name;
  std::string m_description;
};

}