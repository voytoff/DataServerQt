#pragma once

#include "frame.h"

namespace qds
{

class IArchiveWriter
{
public:
  virtual ~IArchiveWriter() noexcept = default;

  virtual bool write(
    const Frame& frame) = 0;
};

}