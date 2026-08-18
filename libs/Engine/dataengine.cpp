#include "dataengine.h"

namespace qds
{

bool DataEngine::initialize(
  DataSourceManager& dataSources,
  SignalProcessor& signalProcessor,
  BufferManager& buffers,
  IArchiveWriter& archive,
  IFramePublisher& publisher,
  ISchedulerClock& clock) noexcept
{
  m_dataSources = &dataSources;
  m_signalProcessor = &signalProcessor;

  m_buffers = &buffers;

  m_archive = &archive;
  m_publisher = &publisher;

  m_clock = &clock;

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
    // TODO: регистрация ошибки архива / recovery state
  }

  return true;
}

void DataEngine::stop() noexcept
{
  m_running = false;
}

bool DataEngine::isRunning() const noexcept
{
  return m_running;
}

}