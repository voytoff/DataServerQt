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
  std::size_t frameCount, double frameRate)
{
  if (values.size() != channelCount * frameCount)
    return;

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

    m_deque.push_back(std::move(block));
  }

  m_condition.notify_one();
}

bool DataBlockQueue::pop(
  DataBlock& block)
{
  std::lock_guard lock(m_mutex);

  if (m_deque.empty())
    return false;

  block =
    std::move(m_deque.front());

  m_deque.pop_front();

  return true;
}

bool DataBlockQueue::waitPop(
  DataBlock& block)
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

  block = std::move(m_deque.front());
  m_deque.pop_front();

  return true;
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
  return m_deque.size();
}

}