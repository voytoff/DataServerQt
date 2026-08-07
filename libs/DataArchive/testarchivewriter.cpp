#include "testarchivewriter.h"

namespace  qds
{

bool TestArchiveWriter::write(const Frame &frame)
{
  ++count;

  m_last = frame;

  return true;
}

const Frame &TestArchiveWriter::last() const
{
  return m_last;
}

}
