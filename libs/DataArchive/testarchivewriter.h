#pragma once

#include "iarchivewriter.h"
#include <cstdint>

namespace  qds
{

class TestArchiveWriter : public IArchiveWriter
{
public:

  bool write(const Frame& frame) override;

  uint32_t count = 0;

  const Frame& last() const;

private:

  Frame m_last;
};

}

