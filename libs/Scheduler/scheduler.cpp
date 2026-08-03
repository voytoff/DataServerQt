#include "scheduler.h"

namespace qds
{

Scheduler::Scheduler(
  SchedulerClock &clock,
  BufferManager &buffer,
  IDataSource &dataSource,
  IProcessingGraph &processing,
  IArchive &archive,
  IPublisher &publisher)
  : m_clock(clock)
  , m_buffer(buffer)
{

}

void Scheduler::tick()
{
  /*
  Frame& frame = buffer.beginWrite();

  frame.number = clock.nextFrame();

  frame.timestamp = clock.timestamp();

  frame.wallTime = clock.wallTime();

  dataSource.read(frame);

  processing.process(frame);

  buffer.publish();

  archive.process(buffer.readFrame());

  publisher.process(buffer.readFrame());
  */
}

}