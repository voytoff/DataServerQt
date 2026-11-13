#pragma once

#include "datatypes.h"

#include <string>

namespace qds
{

struct MeasurementUnit
{
  MeasurementUnitId id;
  std::string name;
  std::string symbol;
};

}