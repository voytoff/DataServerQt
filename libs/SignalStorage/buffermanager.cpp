#include "buffermanager.h"

namespace qds
{

void BufferManager::initialize(
  const SignalMemoryLayout& layout)
{
  std::lock_guard lock(m_mutex);

  m_frame.initialize(layout);
  m_ready = false;
}

void BufferManager::publish(
  const Frame& frame)
{
  std::lock_guard lock(m_mutex);

  m_frame = frame;
  m_ready = true;
}

bool BufferManager::readFrame(
  Frame& frame) const
{
  std::lock_guard lock(m_mutex);

  if (!m_ready)
    return false;

  frame = m_frame;
  return true;
}

bool BufferManager::ready() const noexcept
{
  std::lock_guard lock(m_mutex);
  return m_ready;
}

}