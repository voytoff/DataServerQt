#pragma once

#include "buffermanager.h"
#include "iframepublisher.h"

namespace qds
{

class DataEngine
{
public:

  [[nodiscard]]
  bool initialize(
    BufferManager& buffers,
    IFramePublisher& publisher) noexcept;

  [[nodiscard]]
  bool process() noexcept;

  void stop() noexcept;

  [[nodiscard]]
  bool isRunning() const noexcept;

private:

  BufferManager* m_buffers = nullptr;
  IFramePublisher* m_publisher = nullptr;

  bool m_initialized = false;
  bool m_running = false;
};

}