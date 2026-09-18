#pragma once

#include "datastreamanchor.h"

namespace qds
{

class IDataStreamEventSink
{
public:
  virtual ~IDataStreamEventSink() noexcept = default;

  virtual void startStream(
    const DataStreamAnchor& anchor) = 0;
};

}