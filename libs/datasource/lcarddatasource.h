#pragma once

#include "iclock.h"
#include "idatablocksink.h"
#include "idatasource.h"
#include "idatastreameventsink.h"
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
    ModuleId moduleId,
    uint32_t channelCount,
    std::unique_ptr<ILCardModule> module,
    IClock& clock,
    IDataBlockSink* blockSink = nullptr,
    IDataStreamEventSink* eventSink = nullptr);

  ~LCardDataSource() noexcept override;

  bool start() noexcept override;
  void stop() noexcept override;

  bool acquire(std::span<double> values) override;

private:
  void run() noexcept;

private:
  ModuleId m_moduleId;

  std::unique_ptr<ILCardModule> m_module;
  uint32_t m_channelCount;
  uint64_t m_nextFrameIndex = 0;

  std::vector<double> m_values;
  std::vector<double> m_work;

  std::thread m_thread;
  std::atomic<bool> m_running{false};

  std::mutex m_valuesMutex;

  IClock& m_clock;

  IDataBlockSink* m_blockSink = nullptr;
  IDataStreamEventSink* m_eventSink = nullptr;
};

}