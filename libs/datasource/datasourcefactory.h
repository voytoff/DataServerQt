#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "idatasource.h"
#include "moduleconfiguration.h"

namespace qds
{
  // registerType() возвращает false, если такой ModuleType уже зарегистрирован; false, если creator пустой;
  // create() возвращает nullptr, если тип не зарегистрирован;
  // create() передаёт всю ModuleConfiguration зарегистрированному creator'у;
  // JSON разбирает уже creator/конкретный Settings, а не сама фабрика.
class DataSourceFactory
{
public:
  using Creator =
    std::function<std::unique_ptr<IDataSource>(
      const ModuleConfiguration&)>;

  bool registerType(
    ModuleType type,
    Creator creator);

  [[nodiscard]]
  std::unique_ptr<IDataSource> create(
    const ModuleConfiguration& configuration) const;

  template<class T, class Settings>
  void registerType(ModuleType type)
  {
    registerType(
      type,
      [](const ModuleConfiguration& config)
      {
        auto settings =
          Settings::fromJson(config.settings);

        return std::make_unique<T>(settings);
      });
  }


private:
  std::unordered_map<ModuleType, Creator> m_creators;

};

}
