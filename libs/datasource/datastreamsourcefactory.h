#pragma once

#include "idatastreamsource.h"
#include "moduleruntimeconfiguration.h"
#include <functional>
#include <memory>
#include <unordered_map>

namespace qds
{

class DataStreamSourceFactory
{
public:

  using Creator =
    std::function<
      std::unique_ptr<IDataStreamSource>(
        const ModuleRuntimeConfiguration&)>;

  bool registerType(
    ModuleType type,
    Creator creator);

  std::unique_ptr<IDataStreamSource> create(
    const ModuleRuntimeConfiguration& configuration) const;

private:

  std::unordered_map<ModuleType, Creator> m_creators;
};

}
