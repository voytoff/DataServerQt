#include "fakelcardmodule.h"

namespace qds
{

FakeLCardModule::FakeLCardModule(
  std::size_t channelCount,
  std::size_t blockFrameCount)
  : m_channelCount(channelCount),
  m_blockFrameCount(blockFrameCount)
{
}

bool FakeLCardModule::start() noexcept
{
  if (m_running)
    return false;

  ++startCalls;
  m_running = true;

  return true;
}

void FakeLCardModule::stop() noexcept
{
  ++stopCalls;
  m_running = false;
}

std::size_t
FakeLCardModule::blockFrameCapacity() const noexcept
{
  return m_blockFrameCount;
}

std::size_t FakeLCardModule::readBlock(
  std::span<double> values) noexcept
{
  if (!m_running)
    return 0;

  const std::size_t valueCount =
    m_channelCount * m_blockFrameCount;

  if (values.size() < valueCount)
    return 0;

  ++readCalls;

  for (std::size_t n = 0; n < valueCount; ++n)
    values[n] = m_counter++;

  return m_blockFrameCount;
}

}