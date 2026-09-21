#include "lcarddatasource.h"

#include <algorithm>
#include <cassert>

namespace qds
{

LCardDataSource::LCardDataSource(
  ModuleId moduleId,
  uint32_t channelCount,
  std::unique_ptr<ILCardModule> module,
  IClock &clock,
  IDataBlockSink *blockSink,
  IDataStreamEventSink *eventSink)
  : m_moduleId(moduleId)
  , m_module(std::move(module))
  , m_channelCount(channelCount)
  , m_values(channelCount, 0.0)
  , m_clock(clock)
  , m_blockSink(blockSink)
  , m_eventSink(eventSink)
{
  if (m_module)
  {
    m_work.resize(
      channelCount *
        m_module->blockFrameCapacity(),
      0.0);
  }
}


LCardDataSource::~LCardDataSource() noexcept
{
  stop();
}

bool LCardDataSource::start() noexcept
{
  if (m_running)
    return true;

  if (!m_module)
    return false;

  const Timestamp startTimestamp =
    m_clock.timestamp();

  const WallClockTime startWallTime =
    m_clock.wallClockTime();

  if (!m_module->start())
    return false;

  if (m_eventSink)
  {
    DataStreamAnchor anchor;

    anchor.module = m_moduleId;
    anchor.firstFrameIndex = m_nextFrameIndex;
    anchor.startTimestamp = startTimestamp;
    anchor.startWallTime = startWallTime;
    anchor.frameRate = m_module->frameRate();

    m_eventSink->startStream(anchor);
  }

  m_running = true;

  try
  {
    m_thread =
      std::thread(
        &LCardDataSource::run,
        this);
  }
  catch (...)
  {
    m_running = false;
    m_module->stop();

    return false;
  }

  return true;
}


void LCardDataSource::stop() noexcept
{
  if (!m_running)
    return;

  m_running = false;

  if (m_thread.joinable())
    m_thread.join();

  m_module->stop();
}


bool LCardDataSource::acquire(
  std::span<double> values)
{
  if (!m_running)
    return false;

  if (values.size() != m_values.size())
    return false;

  std::lock_guard lock(m_valuesMutex);

  std::ranges::copy(
    m_values,
    values.begin());

  return true;
}

void LCardDataSource::run() noexcept
{
  while (m_running)
  {
    const std::size_t frameCount =
      m_module->readBlock(m_work);

    if (frameCount == 0)
      continue;

    assert(
      frameCount <=
      m_module->blockFrameCapacity());

    assert(
      frameCount * m_channelCount <=
      m_work.size());

    const uint64_t firstFrameIndex =
      m_nextFrameIndex;

    m_nextFrameIndex += frameCount;

    if (m_blockSink)
    {
      m_blockSink->push(
        m_moduleId,
        firstFrameIndex,
        std::span(
          m_work.data(),
          frameCount * m_channelCount),
        m_channelCount,
        frameCount,
        m_module->frameRate());
    }

    const std::size_t offset =
      (frameCount - 1) * m_channelCount;

    {
      std::lock_guard lock(m_valuesMutex);

      std::copy_n(
        m_work.data() + offset,
        m_channelCount,
        m_values.data());
    }
  }
}

}