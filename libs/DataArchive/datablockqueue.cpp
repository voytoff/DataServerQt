#include "datablockqueue.h"

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <span>

namespace qds
{

void DataBlockQueue::push(
  ModuleId module,
  uint64_t firstFrameIndex,
  std::span<const double> values,
  std::size_t channelCount,
  std::size_t frameCount,
  double frameRate)
{
  DataBlock block;

  block.module = module;
  block.firstFrameIndex = firstFrameIndex;
  block.channelCount = channelCount;
  block.frameCount = frameCount;
  block.frameRate = frameRate;

  block.values.assign(
    values.begin(),
    values.end());

  {
    std::lock_guard lock(m_mutex);

    if (m_stopped)
      return;

    m_deque.emplace_back(
      std::move(block));
  }

  m_condition.notify_one();
}

void DataBlockQueue::startStream(
  const DataStreamAnchor& anchor)
{
  {
    std::lock_guard lock(m_mutex);

    if (m_stopped)
      return;

    m_deque.emplace_back(anchor);
  }

  m_condition.notify_one();
}

bool DataBlockQueue::pop(
  DataStreamEvent& event)
{
  std::lock_guard lock(m_mutex);

  if (m_deque.empty())
    return false;

  event = std::move(m_deque.front());
  m_deque.pop_front();

  return true;
}

bool DataBlockQueue::waitPop(
  DataStreamEvent& event)
{
  std::unique_lock lock(m_mutex);

  m_condition.wait(
    lock,
    [this]
    {
      return m_stopped ||
             !m_deque.empty();
    });

  if (m_deque.empty())
    return false;

  event = std::move(m_deque.front());
  m_deque.pop_front();

  return true;
}

void DataBlockQueue::start() noexcept
{
  std::lock_guard lock(m_mutex);

  m_deque.clear();
  m_stopped = false;
}

void DataBlockQueue::stop() noexcept
{
  {
    std::lock_guard lock(m_mutex);
    m_stopped = true;
  }

  m_condition.notify_all();
}

std::size_t DataBlockQueue::size() noexcept
{
  std::lock_guard lock(m_mutex);
  return m_deque.size();
}

}