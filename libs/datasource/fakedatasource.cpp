#include "fakedatasource.h"

namespace qds
{

bool FakeDataSource::acquire(RawMemory &memory)
{
  memory.setValue(0, 123.0);
  return true;
}

}