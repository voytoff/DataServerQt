#include "teststreamingdatasource.h"

#include <chrono>

namespace qds
{

namespace
{

constexpr double FrameRate = 1000.0;

constexpr uint64_t StartTimestamp = 1'000'000;
constexpr int64_t StartWallTime = 2'000'000;

}

TestStreamingDataSource::TestStreamingDataSource(
  const ModuleRuntimeConfiguration& configuration,
  IDataBlockSink& blockSink,
  IDataStreamEventSink& eventSink)
  : m_module(configuration.module.id)
  , m_channelCount(configuration.tags.size())
  , m_blockSink(blockSink)
  , m_eventSink(eventSink)
{
}

TestStreamingDataSource::~TestStreamingDataSource() noexcept
{
  stop();
}

bool TestStreamingDataSource::start() noexcept
{
  if (m_running.exchange(true))
    return true;

  if (m_thread.joinable())
    m_thread.join();

  try
  {
    m_thread =
      std::thread(
        &TestStreamingDataSource::run,
        this);
  }
  catch (...)
  {
    m_running.store(false);
    return false;
  }

  return true;
}

void TestStreamingDataSource::stop() noexcept
{
  m_running.store(
    false,
    std::memory_order_release);

  if (m_thread.joinable())
    m_thread.join();
}

bool TestStreamingDataSource::isRunning() const noexcept
{
  return m_running.load(
    std::memory_order_acquire);
}

void TestStreamingDataSource::run() noexcept
{
  DataStreamAnchor anchor;

  anchor.module = m_module;
  anchor.firstFrameIndex = 0;
  anchor.startTimestamp =
    Timestamp{StartTimestamp};

  anchor.startWallTime =
    WallClockTime{StartWallTime};

  anchor.frameRate = FrameRate;

  m_eventSink.startStream(anchor);

  std::vector<double> values(
    m_channelCount);

  uint64_t frameIndex = 0;

  while (m_running.load(
    std::memory_order_acquire))
  {
    for (std::size_t channel = 0;
         channel < m_channelCount;
         ++channel)
    {
      values[channel] =
        static_cast<double>(
          frameIndex * 10 + channel);
    }

    m_blockSink.push(
      m_module,
      frameIndex,
      values,
      m_channelCount,
      1,
      FrameRate);

    ++frameIndex;

    std::this_thread::sleep_for(
      std::chrono::milliseconds(1));
  }
}

}