#pragma once

#include "frame.h"
#include "signalmemorylayout.h"

#include <mutex>

namespace qds
{

class BufferManager
{
public:

  void initialize(
    const SignalMemoryLayout& layout);

  void publish(
    const Frame& frame);

  [[nodiscard]]
  bool readFrame(
    Frame& frame) const;

  [[nodiscard]]
  bool ready() const noexcept;

private:

  mutable std::mutex m_mutex;

  Frame m_frame;
  bool m_ready = false;
};

}