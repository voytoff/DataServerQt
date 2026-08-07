#pragma once

#include "iarchivewriter.h"

namespace qds
{
class FailingArchiveWriter : public IArchiveWriter
{
public:

  bool write(const Frame&) override
  {
    return false;
  }
};

}