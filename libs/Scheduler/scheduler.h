#pragma once

#include "buffermanager.h"
#include "idatasource.h"
#include "isignalprocessor.h"

namespace qds
{

class Scheduler
{
public:
  Scheduler(IDataSource& source,
            ISignalProcessor& processor,
            BufferManager& buffers);

  void tick();

private:
  IDataSource& m_source;
  ISignalProcessor& m_processor;
  BufferManager& m_buffers;
};

}