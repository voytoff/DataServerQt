#include "scheduler.h"
#include "schedulerclock.h"

namespace qds
{


Scheduler::Scheduler(
  BufferManager &buffers,
  ICalculationProcessor &processor,
  SchedulerClock &clock)
  : m_buffers(buffers)
  , m_processor(processor)
  , m_clock(clock) { }

void Scheduler::addDataSource(IDataSource &source)
{
  m_sources.push_back(source);
}

bool Scheduler::tick()
{
  Frame& frame = m_buffers.beginWrite();

  m_clock.nextTick();

  frame.number    = m_clock.frameNumber();
  frame.timestamp = m_clock.timestamp();
  frame.wallTime  = m_clock.wallClockTime();

  for (IDataSource& source : m_sources)
  {
    if (!source.acquire(frame.raw().values()))
      return false;
  }

  if (!m_processor.process(frame))
    return false;

  m_buffers.publish();

  return true;
}

}