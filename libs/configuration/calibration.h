#pragma once

#include "signaldefinition.h"
#include "calibrationkind.h"
#include <QString>

namespace qds
{

enum class SensorCalibrationMode
{
  Own = 0,
  ByType = 1
};

struct CalibrationPoint
{
  uint32_t index;
  double x;
  double y;
};

struct CalibrationSegment
{
  double x0 = 0.0;
  double y0 = 0.0;
  double k = 0.0;
};

struct Calibration
{
  CalibrationId id;

  SensorId sensorId;
  SensorTypeId sensorTypeId;

  CalibrationKind kind = CalibrationKind::Table;

  QString name;
  QString description;

  std::vector<CalibrationPoint> points;
  std::vector<CalibrationSegment> segments;

  std::vector<double> coefficients;

  bool buildSegments();

  bool apply(
    double x,
    double& y) const;
};

}