#include "generatordatasource.h"
#include "countergenerator.h"
#include <algorithm>

namespace qds
{

GeneratorDataSource::GeneratorDataSource(
  IModuleDataSink& sink,
  const SystemConfiguration& cfg, IClock &clock)
  : m_sink(sink)
  , m_cfg(cfg)
  , m_clock(clock)
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
    if (!generateModule(module, timestamp))
    {
      return false;
    }
  }

  ++m_generationCount;
  return true;
}

bool GeneratorDataSource::step()
{
  return generateOnce(m_clock.now());
}

uint64_t GeneratorDataSource::generationCount() const noexcept
{
  return m_generationCount;
}

bool GeneratorDataSource::setGenerator(
  ModuleId module,
  std::unique_ptr<IModuleGenerator> gen)
{
  auto it = std::find_if(
    m_modules.begin(),
    m_modules.end(),
    [module](const ModuleContext& m)
    {
      return m.id == module;
    });

  if (it == m_modules.end())
    return false;

  it->generator = std::move(gen);
  return true;
}

bool GeneratorDataSource::generateModule(
  ModuleContext &module,
  uint64_t timestamp)
{
  const auto& tags =
    m_cfg.moduleTags(module.id);

  m_buffer.resize(tags.size());

  module.generator->generate(module.id, m_buffer);

  return m_sink.updateModule(
    module.id,
    m_buffer,
    timestamp);
}

}
