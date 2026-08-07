#include "dataengine.h"

namespace qds
{

bool DataEngine::initialize(
  IDataSource &source,
  ICalculationProcessor &processor,
  BufferManager &buffers,
  IArchiveWriter &archive,
  IFramePublisher &publisher,
  ISchedulerClock &clock)
{
  m_source = &source;
  m_processor = &processor;
  m_buffers = &buffers;
  m_archive = &archive;
  m_publisher = &publisher;
  m_clock = &clock;

  return true;
}

bool DataEngine::process()
{
  Frame& frame = m_buffers->beginWrite();

  m_clock->nextTick();

  frame.number =
    m_clock->frameNumber();

  frame.timestamp =
    m_clock->timestamp();

  frame.wallTime =
    m_clock->wallClockTime();


  if (!m_source->acquire(frame.raw()))
    return false;

  if (!m_processor->process(frame))
    return false;

  m_buffers->publish();

  const Frame& published = m_buffers->readFrame();

  auto ok = m_archive->write(published);

  m_publisher->publish(published);

  return ok;
}


}