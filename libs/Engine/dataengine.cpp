#include "dataengine.h"

namespace qds
{

bool DataEngine::initialize(
  BufferManager& buffers,
  IFramePublisher& publisher) noexcept
{
  m_buffers = &buffers;
  m_publisher = &publisher;

  m_initialized = true;
  m_running = true;

  return true;
}

bool DataEngine::process() noexcept
{
  if (!m_initialized ||
      !m_running)
  {
    return false;
  }

  Frame frame;

  if (!m_buffers->readFrame(frame))
    return true;

  m_publisher->publish(frame);

  return true;
}

void DataEngine::stop() noexcept
{
  if (!m_initialized)
    return;

  m_running = false;
  m_initialized = false;
}

bool DataEngine::isRunning() const noexcept
{
  return m_running;
}

}