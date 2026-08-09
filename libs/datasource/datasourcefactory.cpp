#include "datasourcefactory.h"

namespace qds
{

std::unique_ptr<IDataSource>
DataSourceFactory::create(
  const ModuleConfiguration& configuration) const
{
  const auto type =
    configuration.module.type;

  if (type == ModuleType::Unknown)
    return nullptr;

  auto it = m_creators.find(type);

  if (it == m_creators.end())
    return nullptr;

  return it->second(configuration);
}

bool DataSourceFactory::registerType(
  ModuleType type,
  Creator creator)
{
  if (type == ModuleType::Unknown)
    return false;

  if (!creator)
    return false;

  auto [it, inserted] =
    m_creators.emplace(
      type,
      std::move(creator));

  return inserted;
}

}