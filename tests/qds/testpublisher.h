#pragma once

#include "iframepublisher.h"

namespace qds
{

class TestPublisher final : public IFramePublisher
{
public:

  void publish(
    const Frame& frame) override
  {
    m_frames.push_back(frame);
  }

  const Frame* front() const noexcept
  {
    if (m_frames.empty())
      return nullptr;

    return &m_frames.front();
  }

  const Frame* last() const noexcept
  {
    if (m_frames.empty())
      return nullptr;

    return &m_frames.back();
  }

  std::size_t size() const noexcept
  {
    return m_frames.size();
  }

private:

  std::vector<Frame> m_frames;
};

}
