#include "testpublisher.h"

namespace qds
{

void TestPublisher::publish(const Frame & frame)
{
  ++count;

  m_last = frame;

}

const Frame &TestPublisher::last() const
{
  return m_last;
}

}