#ifndef MODULEDATASOURCE_H
#define MODULEDATASOURCE_H

#include "iclock.h"
#include "idatasource.h"
#include "imoduledevice.h"
#include "imoduledatasink.h"
#include "systemconfiguration.h"

namespace qds
{

class ModuleDataSource : public IDataSource
{
public:
  ModuleDataSource(
    IModuleDataSink& sink,
    const SystemConfiguration& cfg,
    ModuleId module,
    IModuleDevice& device,
    IClock& clock);

  bool start() override;
  void stop() noexcept override;

  bool step() override;

  bool isRunning() const noexcept override;

private:
  bool m_running = false;

  const SystemConfiguration& m_cfg;
  IModuleDataSink& m_sink;
  ModuleId m_module;
  IModuleDevice& m_device;
  IClock& m_clock;

  std::vector<float> m_buffer;
};

}

#endif // MODULEDATASOURCE_H
