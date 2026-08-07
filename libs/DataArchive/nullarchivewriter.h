#pragma once

#include "iarchivewriter.h"

namespace qds
{

class NullArchiveWriter : public IArchiveWriter
{
public:
  bool write(
    const Frame&) override;
};

}
