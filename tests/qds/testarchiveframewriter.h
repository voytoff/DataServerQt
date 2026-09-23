#pragma once

#include "frame.h"
#include "iarchivewriter.h"

namespace qds
{

class TestArchiveFrameWriter final
  : public IArchiveWriter
{
public:
  bool write(
    const Frame& frame)
  {
    frames.push_back(frame);
    return true;
  }

  std::vector<Frame> frames;
};

}