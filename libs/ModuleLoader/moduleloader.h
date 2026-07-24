#ifndef MODULELOADER_H
#define MODULELOADER_H

#include "datasourcemanager.h"
#include "iclock.h"
#include "ihardwaremodulefactory.h"
#include "imoduledatasink.h"
#include "systemconfiguration.h"

namespace qds
{

class ModuleLoader
{
public:
  ModuleLoader(
    const SystemConfiguration& cfg,
    IModuleDataSink& sink,
    IClock& clock,
    IHardwareModuleFactory& factory,
    DataSourceManager& manager);

  [[nodiscard]]
  bool load();

private:
  const SystemConfiguration& m_cfg;
  IModuleDataSink& m_sink;
  IClock& m_clock;
  DataSourceManager& m_manager;
  IHardwareModuleFactory& m_factory;

};

}

#endif // MODULELOADER_H
