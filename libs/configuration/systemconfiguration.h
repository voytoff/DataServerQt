#ifndef SYSTEMCONFIGURATION_H
#define SYSTEMCONFIGURATION_H

#include <vector>
#include "taginfo.h"
#include "moduleinfo.h"
#include "crateinfo.h"
#include "datatypes.h"

namespace qds
{

class SystemConfiguration
{
public:
  void addCrate(const CrateInfo& crate);
  void addModule(const ModuleInfo& module);
  void addTag(const TagInfo& tag);

  const std::vector<CrateInfo>& crates() const;
  const std::vector<ModuleInfo>& modules() const;
  const std::vector<TagInfo>& tags() const;

  const std::vector<TagId>& moduleTags(ModuleId id) const;

private:

  std::vector<CrateInfo>  m_crates;
  std::vector<ModuleInfo> m_modules;
  std::vector<TagInfo>    m_tags;
  std::vector<std::vector<TagId>> m_moduleTags;
};

}

#endif // SYSTEMCONFIGURATION_H
