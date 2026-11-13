#pragma once

#include "datatypes.h"

namespace qds
{

struct SignalTypeInfo
{
  SignalTypeId id = {0};

  std::string name;
  std::string description;

  MeasurementUnitId measurementUnitId;
};

}
