#pragma once

#include "iframepublisher.h"

namespace qds
{

class TestPublisher : public IFramePublisher
{
public:
  void publish(
    const Frame&frame) override;

  uint32_t count = 0;

  const Frame& last() const;

private:

  Frame m_last;
};

}
