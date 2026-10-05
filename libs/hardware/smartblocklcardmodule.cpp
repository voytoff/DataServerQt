#include "smartblocklcardmodule.h"
#include <cassert>
#include <thread>
#include <chrono>

namespace qds
{

SmartBlockLCardModule::SmartBlockLCardModule(
  const std::size_t frameCount,
  const std::size_t channelCount,
  const uint32_t blockCount,
  const std::optional<uint32_t> successCount)
  : m_frameCount(frameCount)
  , m_channelCount(channelCount)
  , m_successCount(successCount)
  , m_blockCount(blockCount)
{
}

bool SmartBlockLCardModule::start() noexcept
{
  m_running.store(true);
  return true;
}

void SmartBlockLCardModule::stop() noexcept
{
  ++stopCalls;
  m_running.store(false);
}

std::size_t SmartBlockLCardModule::blockFrameCapacity() const noexcept
{
  return m_frameCount;
}

double SmartBlockLCardModule::frameRate() const noexcept
{
  return 1000;
}

uint32_t SmartBlockLCardModule::remainingBlockCount() const noexcept
{
  return m_blockCount.load();
}

LCardReadResult SmartBlockLCardModule::readBlock(
  std::span<double> values) noexcept
{
  if (!m_running.load())
    return {};

  auto successCount = m_successCount.load();
  if (
    successCount.has_value() &&
    successCount.value() == 0)
  {
    return {
      .status = LCardReadStatus::Error,
      .frameCount = 0
    };
  }

  const std::size_t valueCount =
    m_frameCount *
    m_channelCount;

  assert(
    values.size() >= valueCount);

  if (values.size() < valueCount)
    return {};

  if (m_blockCount.load() == 0)
  {
    std::this_thread::sleep_for(
      std::chrono::milliseconds(1));

    return {
      .status = LCardReadStatus::NoData,
      .frameCount = 0
    };
  }

  m_blockCount.fetch_sub(1);

  for (std::size_t i = 0;
       i < valueCount;
       ++i)
  {
    values[i] =
      static_cast<double>(i);
  }

  successCount = m_successCount.load();
  if (successCount.has_value())
    m_successCount.store(--successCount.value());

  return {
    .status = LCardReadStatus::Data,
    .frameCount = m_frameCount
  };
}

void SmartBlockLCardModule::setBlockCount(
  const uint32_t blockCount)
{
  m_blockCount.store(blockCount);
}

void SmartBlockLCardModule::setSuccessCount(
  const std::optional<uint32_t> successCount)
{
  m_successCount.store(successCount);
}

}