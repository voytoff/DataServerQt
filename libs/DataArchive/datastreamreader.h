#pragma once

#include "datastreamevent.h"
#include "datastreamframe.h"
#include "datastreamtime.h"

#include <vector>

namespace qds
{

class DataStreamReader
{
public:
  bool process(
    const DataStreamEvent& event,
    std::vector<DataStreamFrame>& frames);

private:
  DataStreamTime m_time;
};

}