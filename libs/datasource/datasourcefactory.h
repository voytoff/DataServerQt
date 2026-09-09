#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "idatasource.h"
#include "moduleruntimeconfiguration.h"
#include "moduletype.h"

namespace qds
{

class DataSourceFactory
{
public:
  using Creator =
    std::function<std::unique_ptr<IDataSource>(
      const ModuleRuntimeConfiguration&)>;

  bool registerType(ModuleType type, Creator creator);

  [[nodiscard]]
  std::unique_ptr<IDataSource> create(
    const ModuleRuntimeConfiguration& configuration) const;

  /*
  template<class T, class Settings>
  void registerType(ModuleType type)
  {
    registerType(
      type,
      [](const ModuleConfiguration& config, uint32_t channelCount)
      {
        auto settings =
          Settings::fromJson(config.settings);

        return std::make_unique<T>(
          settings,
          channelCount);
      });
  }
  */

private:
  std::unordered_map<ModuleType, Creator> m_creators;
};

}