#pragma once


#include "calibration.h"
#include "datatypes.h"
#include <string>

namespace qds
{

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