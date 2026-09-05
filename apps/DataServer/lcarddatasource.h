#pragma once

#include "idatasource.h"
#include "ilcardmodule.h"
#include "moduleconfiguration.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace qds
{

class LCardDataSource : public IDataSource
{
public:
  explicit LCardDataSource(
    const ModuleConfiguration& configuration,
    std::unique_ptr<ILCardModule> module);

  ~LCardDataSource() noexcept override;

  bool start() noexcept override;
  void stop() noexcept override;

  bool acquire(std::span<double> values) override;

private:
  void run() noexcept;

private:
  const ModuleConfiguration m_configuration;

  std::unique_ptr<ILCardModule> m_module;

  std::vector<double> m_values;

  std::thread m_thread;
  std::atomic<bool> m_running{false};

  std::mutex m_valuesMutex;

  std::mutex m_waitMutex;
  std::condition_variable m_waitCondition;

  std::chrono::microseconds m_pollInterval;
};

}