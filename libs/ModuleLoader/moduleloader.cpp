#include "moduleloader.h"
#include "moduledatasource.h"

namespace qds
{

ModuleLoader::ModuleLoader(
  const SystemConfiguration &cfg,
  IModuleDataSink &sink,
  IClock &clock,
  IHardwareModuleFactory &factory,
  DataSourceManager &manager)
  : m_cfg(cfg)
  , m_sink(sink)
  , m_clock(clock)
  , m_manager(manager)
  , m_factory(factory) {}

bool ModuleLoader::load()
{
  for (const auto& module : m_cfg.modules())
  {
    auto device = m_factory.create(module);

    if (!device)
      return false;

    auto source = std::make_unique<ModuleDataSource>(
      m_sink,
      m_cfg,
      module,
      std::move(device),
      m_clock);

    if (!m_manager.add(std::move(source)))
      return false;
  }

  return true;
}

}