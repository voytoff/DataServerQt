#pragma once

#include "frame.h"

namespace qds
{

class IFramePublisher
{
public:
  virtual ~IFramePublisher() noexcept = default;

  virtual void publish(
    const Frame& frame) = 0;
};

}