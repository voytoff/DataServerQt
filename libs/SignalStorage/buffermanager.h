#pragma once

#include "frame.h"
#include "signalmemorylayout.h"

namespace qds
{

class BufferManager
{
public:

  void initialize(const SignalMemoryLayout& layout);

  Frame& beginWrite();

  void publish();

  const Frame& readFrame() const;

  [[nodiscard]]
  bool ready() const noexcept;

private:
  Frame m_frames[2];

  uint32_t m_writeIndex = 0;
  uint32_t m_readIndex = 1;

  bool m_ready = false;
  bool m_building = false;
};

}
