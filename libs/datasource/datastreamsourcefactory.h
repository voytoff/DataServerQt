#pragma once

#include "iclock.h"
#include "idatablocksink.h"
#include "idatastreameventsink.h"
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
    std::function<std::unique_ptr<IDataStreamSource>(
      const ModuleRuntimeConfiguration&,
      IClock&,
      IDataBlockSink&,
      IDataStreamEventSink&)>;

  bool registerType(
    ModuleType type,
    Creator creator);

  std::unique_ptr<IDataStreamSource> create(
    const ModuleRuntimeConfiguration& configuration,
    IClock& clock,
    IDataBlockSink& blockSink,
    IDataStreamEventSink& eventSink

    ) const;

private:

  std::unordered_map<ModuleType, Creator> m_creators;
};

}
