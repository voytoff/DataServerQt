#include "scheduler.h"
#include "idatasource.h"

namespace qds
{

Scheduler::Scheduler(
  IDataSource &source,
  ISignalProcessor &processor,
  BufferManager &buffers)
  : m_source(source)
  , m_processor(processor)
  , m_buffers(buffers)
{

}

void Scheduler::tick()
{
  Frame& frame = m_buffers.beginWrite();

  m_source.read(frame.rawMemory);

  m_processor.process(frame);

  frame.number = ...;
  frame.timestamp = ...;
  frame.wallTime = ...;

  m_buffers.publish();
}

}