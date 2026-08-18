#include "failoncearchivewriter.h"

namespace qds
{

bool FailOnceArchiveWriter::write(const Frame &frame)
{
  ++attempts;

  if (fail)
  {
    fail = false;
    return false;
  }

  last = frame;
  ++successes;
  return true;
}

}
