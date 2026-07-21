#include "generatordatasource.h"
#include "countergenerator.h"
#include <chrono>

namespace qds
{

GeneratorDataSource::GeneratorDataSource(
  IModuleDataSink& sink,
  const SystemConfiguration& cfg)
  : m_sink(sink)
  , m_cfg(cfg)
{
  for (const auto& module : cfg.modules())
  {
    m_modules.emplace_back(
      module.id,
      std::make_unique<CounterGenerator>());
  }
}

bool GeneratorDataSource::start()
{
  if (m_running)
    return false;

  m_running = true;
  return true;
}

void GeneratorDataSource::stop() noexcept
{
  m_running = false;
}

bool GeneratorDataSource::isRunning() const noexcept
{
  return m_running;
}

bool GeneratorDataSource::generateOnce(uint64_t timestamp)
{
  for (auto& module : m_modules)
  {
    if (!generateModule(module.id,
                        *module.generator,
                        timestamp))
    {
      return false;
    }
  }

  ++m_generationCount;
  return true;
}

bool GeneratorDataSource::step()
{
  const auto now =
    std::chrono::system_clock::now();

  const uint64_t timestamp =
    std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch())
      .count();
  return generateOnce(timestamp);
}

uint64_t GeneratorDataSource::generationCount() const noexcept
{
  return m_generationCount;
}

bool GeneratorDataSource::generateModule(
  ModuleId module,
  IModuleGenerator& generator,
  uint64_t timestamp)
{
  //const auto& tags = m_cfg.moduleTags(module);

  m_buffer.resize(m_cfg.moduleTags(module).size());

  generator.generate(module, m_buffer);

  return m_sink.updateModule(
    module,
    m_buffer,
    timestamp);
}

}
