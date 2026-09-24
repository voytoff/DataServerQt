#include "datastreamworker.h"

namespace qds
{

DataStreamWorker::DataStreamWorker(
  DataBlockQueue &queue,
  DataStreamProcessor &processor,
  ILogger &logger)
  : m_queue(queue)
  , m_processor(processor)
  , m_logger(logger)
{
}

DataStreamWorker::~DataStreamWorker() noexcept
{
  m_queue.stop();
  join();
}

bool DataStreamWorker::start() noexcept
{
  if (m_running.exchange(true))
    return true;

  try
  {
    m_thread =
      std::thread(
        &DataStreamWorker::run,
        this);
  }
  catch (...)
  {
    m_running.store(false);
    return false;
  }

  return true;
}

void DataStreamWorker::join() noexcept
{
  if (m_thread.joinable())
    m_thread.join();
}

bool DataStreamWorker::isRunning() const noexcept
{
  return m_running;
}

void DataStreamWorker::run() noexcept
{
  DataStreamEvent event;

  while (m_queue.waitPop(event))
  {
    if (!m_processor.process(event))
    {
      m_logger.error(
        "Data stream processing failed");
    }
  }

  m_running.store(
    false,
    std::memory_order_release);
}

}
