#pragma once

#include <memory>
#include "iclock.h"
#include "iactivedatasource.h"
#include "ilcardmodule.h"
#include "imoduledatasink.h"
#include "systemconfiguration.h"

namespace qds
{

class ModuleDataSource : public IActiveDataSource
{
public:
  ModuleDataSource(
    IModuleDataSink& sink,
    const SystemConfiguration& cfg,
    const ModuleInfo& module,
    std::unique_ptr<IHardwareModule> device,
    IClock& clock);

  bool start() override;
  void stop() noexcept override;

  bool step() override;
  bool isRunning() const noexcept override;

private:
  bool m_running = false;

  const SystemConfiguration& m_cfg;
  IModuleDataSink& m_sink;
  ModuleInfo m_module;
  std::unique_ptr<IHardwareModule> m_device;
  IClock& m_clock;

  std::vector<float> m_buffer;
};

}
