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
    std::span<const double> values,
    std::size_t channelCount,
    std::size_t frameCount) override;

  bool pop(DataBlock& block);

  bool waitPop(DataBlock& block);

  void stop() noexcept;

private:
  std::mutex m_mutex;
  std::condition_variable m_condition;
  std::deque<DataBlock> m_deque;

  bool m_stopped = false;
};

}
