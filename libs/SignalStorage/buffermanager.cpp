#include "buffermanager.h"
#include <cassert>

namespace qds
{

void BufferManager::initialize(const SignalMemoryLayout &layout)
{
  for (Frame &frame : m_frames)
  {
    frame.initialize(layout);
  }
}

Frame& BufferManager::beginWrite()
{
  assert(!m_building);

  m_building = true;
  return m_frames[m_writeIndex];
}

void BufferManager::publish()
{
  assert(m_building);

  m_building = false;
  std::swap(m_writeIndex, m_readIndex);
  m_ready = true;
}

const Frame& BufferManager::readFrame() const
{
  assert(m_ready);

  return m_frames[m_readIndex];
}

bool BufferManager::ready() const noexcept
{
  return m_ready;
}

}