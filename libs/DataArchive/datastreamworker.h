#pragma once

#include "datablockqueue.h"
#include "datastreamprocessor.h"

#include <atomic>
#include <thread>

namespace qds
{

class DataStreamWorker
{
public:

  DataStreamWorker(
    DataBlockQueue& queue,
    DataStreamProcessor& processor,
    ILogger& logger);

  ~DataStreamWorker() noexcept;

  [[nodiscard]]
  bool start() noexcept;

  void join() noexcept;

  [[nodiscard]]
  bool isRunning() const noexcept;

private:

  void run() noexcept;

  ILogger& m_logger;
  DataBlockQueue& m_queue;
  DataStreamProcessor& m_processor;

  std::thread m_thread;
  std::atomic_bool m_running{false};
};

}