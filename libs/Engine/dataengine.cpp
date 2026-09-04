#include "dataengine.h"

namespace qds
{

bool DataEngine::initialize(
  DataSourceManager& dataSources,
  SignalProcessor& signalProcessor,
  BufferManager& buffers,
  IArchiveWriter& archive,
  IFramePublisher& publisher,
  ISchedulerClock& clock,
  ILogger& logger) noexcept
{
  m_dataSources = &dataSources;
  m_signalProcessor = &signalProcessor;
  m_buffers = &buffers;
  m_archive = &archive;
  m_publisher = &publisher;
  m_clock = &clock;
  m_logger = &logger;

  if (!m_dataSources->start())
  {
    m_dataSources = nullptr;
    m_signalProcessor = nullptr;
    m_buffers = nullptr;
    m_archive = nullptr;
    m_publisher = nullptr;
    m_clock = nullptr;
    m_logger = nullptr;

    return false;
  }

  m_initialized = true;
  m_running = true;

  return true;
}

bool DataEngine::process() noexcept
{
  if (!m_initialized || !m_running)
    return false;

  Frame& frame =
    m_buffers->beginWrite();

  m_clock->nextTick();

  frame.number =
    m_clock->frameNumber();

  frame.timestamp =
    m_clock->timestamp();

  frame.wallTime =
    m_clock->wallClockTime();

  if (!m_dataSources->acquire(
        frame.raw()))
  {
    m_buffers->cancelWrite();
    return false;
  }

  if (!m_signalProcessor->process(
        frame.raw(),
        frame.calculated()))
  {
    m_buffers->cancelWrite();
    return false;
  }

  m_buffers->publish();

  const Frame& published =
    m_buffers->readFrame();

  m_publisher->publish(published);

  if (!m_archive->write(published))
  {
    m_logger->error("Archive write failed");
  }

  return true;
}

void DataEngine::stop() noexcept
{
  if (!m_initialized)
    return;

  m_running = false;

  if (m_dataSources)
    m_dataSources->stop();

  m_initialized = false;
}

bool DataEngine::isRunning() const noexcept
{
  return m_running;
}

}