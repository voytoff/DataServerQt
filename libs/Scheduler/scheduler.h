#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "buffermanager.h"
#include "idatasource.h"
#include "schedulerclock.h"

namespace qds
{

class IProcessingGraph
{
public:

  virtual ~IProcessingGraph() = default;

  virtual void process(Frame&) = 0;
};

class IArchive
{
public:

  virtual ~IArchive() = default;

  virtual void process(const Frame&) = 0;
};

class IPublisher
{
public:

  virtual ~IPublisher() = default;

  virtual void process(const Frame&) = 0;
};

class Scheduler
{
public:
  Scheduler(
    SchedulerClock& clock,
    BufferManager& buffer,
    IDataSource& dataSource,
    IProcessingGraph& processing,
    IArchive& archive,
    IPublisher& publisher);

  void tick();

private:
  SchedulerClock& m_clock;
  BufferManager& m_buffer;

};

}

#endif // SCHEDULER_H
