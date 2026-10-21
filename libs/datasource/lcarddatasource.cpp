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
  ILogger &logger,
  IDataBlockSink *blockSink,
  IDataStreamEventSink *eventSink)
  : m_moduleId(moduleId)
  , m_module(std::move(module))
  , m_channelCount(channelCount)
  , m_values(channelCount, 0.0)
  , m_clock(clock)
  , m_logger(logger)
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
  {
    m_logger.error("LCard: модуль не создан, id=" +
      std::to_string(m_moduleId.value));

    return false;
  }

  if (m_thread.joinable())
    m_thread.join();

  if (m_moduleStarted)
  {
    m_module->stop();
    m_moduleStarted = false;
  }

  const Timestamp startTimestamp =
    m_clock.timestamp();

  const WallClockTime startWallTime =
    m_clock.wallClockTime();

  if (!m_module->start())
  {
    m_logger.error("LCard: не удалось запустить модуль, id=" +
      std::to_string(m_moduleId.value));

    return false;
  }

  m_moduleStarted = true;

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

  m_state.store(
    DataStreamSourceState::Running,
    std::memory_order_release);

  try
  {
    m_thread =
      std::thread(
        &LCardDataSource::run,
        this);
  }
  catch (const std::exception& e)
  {
    m_running = false;
    m_module->stop();
    m_moduleStarted = false;

    m_logger.error(
      "LCard: ошибка создания потока модуля, id=" +
      std::to_string(m_moduleId.value) +
      ": " + e.what());

    m_state.store(
      DataStreamSourceState::Error,
      std::memory_order_release);

    return false;
  }
  catch (...)
  {
    m_running = false;
    m_module->stop();
    m_moduleStarted = false;

    m_logger.error(
      "LCard: неизвестная ошибка создания потока модуля, id=" +
      std::to_string(m_moduleId.value));

    m_state.store(
      DataStreamSourceState::Error,
      std::memory_order_release);

    return false;
  }

  return true;
}

void LCardDataSource::stop() noexcept
{
  m_running.store(
    false,
    std::memory_order_release);

  if (m_thread.joinable())
    m_thread.join();

  if (m_module && m_moduleStarted)
  {
    m_module->stop();
    m_moduleStarted = false;
  }

  if (m_state.load(std::memory_order_acquire) !=
      DataStreamSourceState::Error)
  {
    m_state.store(
      DataStreamSourceState::Stopped,
      std::memory_order_release);
  }
}

bool LCardDataSource::isRunning() const noexcept
{
  return m_running.load(
    std::memory_order_acquire);
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

DataStreamSourceStatistics
LCardDataSource::statistics() const noexcept
{
  return {
    .blocksReceived =
    m_blocksReceived.load(std::memory_order_relaxed),

    .framesReceived =
    m_framesReceived.load(std::memory_order_relaxed),

    .lastTimestamp =
    m_lastTimestamp.load(std::memory_order_relaxed)
  };
}

DataStreamSourceState
LCardDataSource::state() const noexcept
{
  return m_state.load(
    std::memory_order_acquire);
}

void LCardDataSource::run() noexcept
{
  bool error = false;

  while (m_running.load(
    std::memory_order_acquire))
  {
    const LCardReadResult result =
      m_module->readBlock(m_work);

    if (result.status == LCardReadStatus::Error)
    {
      m_logger.error(
        "LCard: ошибка чтения данных модуля, id=" +
        std::to_string(m_moduleId.value));

      error = true;
      break;
    }

    if (result.status == LCardReadStatus::NoData)
      continue;

    if (result.frameCount == 0 ||
        result.frameCount > m_module->blockFrameCapacity() ||
        m_channelCount == 0 ||
        result.frameCount > m_work.size() / m_channelCount)
    {
      m_logger.error(
        "LCard: некорректный блок данных модуля, id=" +
        std::to_string(m_moduleId.value));

      error = true;
      break;
    }

    const std::size_t valueCount =
      result.frameCount *
      m_channelCount;

    if (valueCount > m_work.size())
      break;

    const uint64_t firstFrameIndex =
      m_nextFrameIndex;

    m_nextFrameIndex +=
      result.frameCount;

    if (m_blockSink)
    {
      m_blockSink->push(
        m_moduleId,
        firstFrameIndex,
        std::span(
          m_work.data(),
          valueCount),
        m_channelCount,
        result.frameCount,
        m_module->frameRate());
    }

    const std::size_t offset =
      (result.frameCount - 1) *
      m_channelCount;

    {
      std::lock_guard lock(
        m_valuesMutex);

      std::copy_n(
        m_work.data() + offset,
        m_channelCount,
        m_values.data());
    }

    m_blocksReceived.fetch_add(
      1,
      std::memory_order_relaxed);

    m_framesReceived.fetch_add(
      result.frameCount,
      std::memory_order_relaxed);

    m_lastTimestamp.store(
      m_clock.wallClockTime().unixMicroseconds,
      std::memory_order_relaxed);
  }

  m_state.store(
    error
      ? DataStreamSourceState::Error
      : DataStreamSourceState::Stopped,
    std::memory_order_release);

  m_running.store(
    false,
    std::memory_order_release);
}

}