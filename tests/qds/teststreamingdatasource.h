#pragma once

#include "idatastreamsource.h"
#include "idatablocksink.h"
#include "idatastreameventsink.h"
#include "moduleruntimeconfiguration.h"

#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>

namespace qds
{

class TestStreamingDataSource final
  : public IDataStreamSource
{
public:
  TestStreamingDataSource(
    const ModuleRuntimeConfiguration& configuration,
    IDataBlockSink& blockSink,
    IDataStreamEventSink& eventSink);

  ~TestStreamingDataSource() noexcept override;

  [[nodiscard]]
  bool start() noexcept override;

  void stop() noexcept override;

private:
  void run() noexcept;

  ModuleId m_module;
  std::size_t m_channelCount = 0;

  IDataBlockSink& m_blockSink;
  IDataStreamEventSink& m_eventSink;

  std::thread m_thread;
  std::atomic_bool m_running{false};
};

}