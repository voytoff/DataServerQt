#include "testdatasource.h"

namespace qds
{

bool TestDataSource::acquire(RawMemory &memory)
{
  memory.setValue(0, m_counter);
  memory.setValue(1, m_counter * 10);

  ++m_counter;

  return true;
}

}
