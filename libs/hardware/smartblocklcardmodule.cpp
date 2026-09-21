#include "smartblocklcardmodule.h"
#include <cassert>
#include  <thread>

namespace qds
{

SmartBlockLCardModule::SmartBlockLCardModule(
  const std::size_t frameCount,
  const std::size_t channelCount,
  const uint32_t blockCount)
  : m_frameCount(frameCount)
  , m_channelCount(channelCount)
  , m_blockCount(blockCount)
{
}

bool SmartBlockLCardModule::start() noexcept
{
  m_running = true;
  return true;
}

void SmartBlockLCardModule::stop() noexcept
{
  m_running = false;
}

std::size_t SmartBlockLCardModule::blockFrameCapacity() const noexcept
{
  return m_frameCount;
}

double SmartBlockLCardModule::frameRate() const noexcept
{
  return 1000;
}

std::size_t SmartBlockLCardModule::readBlock(
  std::span<double> values) noexcept
{
  if (!m_running)
    return 0;

  if (m_blockCount == 0)
  {
    std::this_thread::sleep_for(
      std::chrono::milliseconds(1));

    return 0;
  }

  const std::size_t valueCount =
    m_frameCount * m_channelCount;

  assert(values.size() >= valueCount);

  if (values.size() < valueCount)
    return 0;

  --m_blockCount;

  for (std::size_t i = 0; i < valueCount; ++i)
    values[i] = static_cast<double>(i);

  return m_frameCount;
}

void SmartBlockLCardModule::setCount(
  const uint32_t blockCount)
{
  m_blockCount = blockCount;
}

}