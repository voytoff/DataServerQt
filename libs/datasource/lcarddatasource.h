#pragma once

#include "idatasource.h"
#include "ilcardmodule.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <span>
#include <thread>
#include <vector>

namespace qds
{

class LCardDataSource final : public IDataSource
{
public:
  LCardDataSource(
    uint32_t channelCount,
    std::unique_ptr<ILCardModule> module);

  ~LCardDataSource() noexcept override;

  bool start() noexcept override;
  void stop() noexcept override;

  bool acquire(std::span<double> values) override;

private:
  void run() noexcept;

private:
  std::unique_ptr<ILCardModule> m_module;

  std::vector<double> m_values;
  std::vector<double> m_work;

  std::thread m_thread;
  std::atomic<bool> m_running{false};

  std::mutex m_valuesMutex;
};

}