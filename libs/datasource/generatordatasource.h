#ifndef GENERATORDATASOURCE_H
#define GENERATORDATASOURCE_H

#include "iclock.h"
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
    ModuleId moduleId,
    std::unique_ptr<IModuleGenerator> generator)
    : id(moduleId)
    , generator(std::move(generator))
  {}
};

class GeneratorDataSource : public IDataSource
{
public:
  GeneratorDataSource(
    IModuleDataSink& sink,
    const SystemConfiguration& cfg,
    IClock& clock);

  bool start() override;
  void stop() noexcept override;

  [[nodiscard]]
  bool isRunning() const noexcept override;

  bool generateOnce(uint64_t timestamp);
  bool step() override;
  [[nodiscard]]
  uint64_t generationCount() const noexcept;

  [[nodiscard]]
  bool setGenerator(ModuleId module, std::unique_ptr<IModuleGenerator> gen);

  template<class T, class... Args>
  [[nodiscard]]
  bool setGenerator(ModuleId module, Args&&... args)
  {
    return setGenerator(
      module,
      std::make_unique<T>(std::forward<Args>(args)...));
  }

private:
  bool generateModule(
    ModuleContext& module,
    uint64_t timestamp);

private:
  bool m_running = false;

  IModuleDataSink& m_sink;
  const SystemConfiguration& m_cfg;
  IClock& m_clock;
  std::vector<ModuleContext> m_modules;
  std::vector<float> m_buffer;
  uint64_t m_generationCount = 0;
};

}

#endif // GENERATORDATASOURCE_H
