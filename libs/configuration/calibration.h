#pragma once

#include "signaldefinition.h"
#include <QString>

namespace qds
{

struct CalibrationPoint
{
  uint32_t index;
  double x;
  double y;
};

struct CalibrationSegment
{
  double x0;
  double y0;
  double k;
};

struct Calibration
{
  CalibrationId id;

  SignalId signalId;
  SignalTypeId signalTypeId;

  QString name;
  QString description;

  std::vector<CalibrationPoint> points;
  std::vector<CalibrationSegment> segments;

  bool apply(
    double x,
    double& y) const;
};

}