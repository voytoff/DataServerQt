#pragma once

#include "iarchivewriter.h"

namespace qds
{

class FailOnceArchiveWriter : public IArchiveWriter
{
public:
  bool write(const Frame& frame) override;

  bool fail = true;
  int attempts = 0;
  int successes = 0;
  Frame last;
};

}
