#pragma once

#include "calibrationkind.h"
#include "datatypes.h"

#include <string>

namespace qds
{

struct SensorType
{
  SensorTypeId id;

  std::string name;
  std::string description;

  CalibrationKind calibrationKind = CalibrationKind::Table;
};

}