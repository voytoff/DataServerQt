#include "datastreamsourcefactory.h"

namespace qds
{

bool DataStreamSourceFactory::registerType(ModuleType type, Creator creator)
{
  if (type == ModuleType::Unknown)
    return false;

  if (!creator)
    return false;

  const auto [it, inserted] =
    m_creators.emplace(type, std::move(creator));

  return inserted;
}

std::unique_ptr<IDataStreamSource> DataStreamSourceFactory::create(
  const ModuleRuntimeConfiguration &configuration) const
{
  const auto type = configuration.module.type;

  if (type == ModuleType::Unknown)
    return nullptr;

  const auto it = m_creators.find(type);

  if (it == m_creators.end())
    return nullptr;

  return it->second(configuration);
}

}
