#pragma once

#include "buffermanager.h"
#include "idatasource.h"
#include "schedulerclock.h"
#include "icalculationprocessor.h"

namespace qds
{

class Scheduler
{
public:
  Scheduler(BufferManager& buffers,
            ICalculationProcessor& processor,
            SchedulerClock& clock);

  void addDataSource(IDataSource& source);

  bool tick();

private:
  BufferManager& m_buffers;
  ICalculationProcessor& m_processor;
  SchedulerClock& m_clock;

  std::vector<std::reference_wrapper<IDataSource>> m_sources;
};

}