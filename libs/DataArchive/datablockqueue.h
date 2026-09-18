#pragma once

#include "datablock.h"
#include "idatablocksink.h"
#include <condition_variable>
#include <deque>
#include <mutex>

namespace qds
{

class DataBlockQueue final : public IDataBlockSink
{
public:
  void push(
    ModuleId module,
    uint64_t firstFrameIndex,
    std::span<const double> values,
    std::size_t channelCount,
    std::size_t frameCount,
    double frameRate) override;

  bool pop(DataBlock& block);

  bool waitPop(DataBlock& block);

  void stop() noexcept;

  std::size_t size() noexcept;

private:
  std::mutex m_mutex;
  std::condition_variable m_condition;
  std::deque<DataBlock> m_deque;

  bool m_stopped = false;
};

}
