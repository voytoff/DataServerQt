#pragma once

#include "iframepublisher.h"

namespace qds
{

class NullFramePublisher : public IFramePublisher
{
public:
  void publish(
    const Frame&) override;

};

}
