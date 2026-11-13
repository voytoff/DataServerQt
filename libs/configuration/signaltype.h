#pragma once

#include "datatypes.h"

#include <string>

namespace qds
{

struct SignalType
{
  SignalTypeId id;

  std::string name;
  std::string description;

  MeasurementUnitId measurementUnitId;
};

}