#ifndef GENERATORDATASOURCE_H
#define GENERATORDATASOURCE_H

#include "imoduledatasink.h"
#include "imodulegenerator.h"
#include "systemconfiguration.h"
#include "idatasource.h"
#include <memory>

namespace qds
{

struct ModuleContext
{
  ModuleId id;
  std::unique_ptr<IModuleGenerator> generator;

  ModuleContext(
    ModuleId id,
    std::unique_ptr<IModuleGenerator> gen)
    : id(id)
    , generator(std::move(gen))
  {
  }
};

class GeneratorDataSource : public IDataSource
{
public:
  GeneratorDataSource(
    IModuleDataSink& sink,
    const SystemConfiguration& cfg);

  bool start() override;
  void stop() noexcept override;

  [[nodiscard]]
  bool isRunning() const noexcept override;

  bool generateOnce(uint64_t timestamp);
  bool step();
  [[nodiscard]]
  uint64_t generationCount() const noexcept;

private:
  bool generateModule(
    ModuleId module,
    IModuleGenerator& generator,
    uint64_t timestamp);

private:
  bool m_running = false;

  IModuleDataSink& m_sink;
  const SystemConfiguration& m_cfg;
  std::vector<ModuleContext> m_modules;
  std::vector<float> m_buffer;
  uint64_t m_generationCount = 0;
};

}

#endif // GENERATORDATASOURCE_H
