#pragma once

#include "sensortype.h"

#include <string>

namespace qds
{

enum class SensorCalibrationMode
{
  Own = 0,
  ByType = 1
};

struct Sensor
{
  SensorId id;
  SensorTypeId sensorTypeId;

  std::string name;
  std::string serial;
  std::string description;

  SensorCalibrationMode calibrationMode =
    SensorCalibrationMode::Own;

  CalibrationKind calibrationKind =
    CalibrationKind::Table;
};

}