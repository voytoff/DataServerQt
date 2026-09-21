#pragma once

#include "datastreamevent.h"
#include "idatablocksink.h"
#include "idatastreameventsink.h"
#include <condition_variable>
#include <deque>
#include <mutex>

namespace qds
{

class DataBlockQueue final
  : public IDataBlockSink
  , public IDataStreamEventSink
{
public:
  void push(
    ModuleId module,
    uint64_t firstFrameIndex,
    std::span<const double> values,
    std::size_t channelCount,
    std::size_t frameCount,
    double frameRate) override;

  void startStream(
    const DataStreamAnchor& anchor) override;

  bool pop(DataStreamEvent& event);
  bool waitPop(DataStreamEvent& event);

  //bool popEvent(DataStreamEvent& event);
  //bool waitPopEvent(DataStreamEvent& event);

  void stop() noexcept;

  std::size_t size() noexcept;

private:
  std::mutex m_mutex;
  std::condition_variable m_condition;
  std::deque<DataStreamEvent> m_deque;

  bool m_stopped = false;

};

}
