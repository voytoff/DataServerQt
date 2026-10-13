#pragma once

#include <strongidhash.h>
#include "calibration.h"
#include "datatypes.h"

#include <map>
#include <unordered_map>
#include <strongidhash.h>


namespace qds
{
/** табличные и полиномиальные тарировки. */
class CalibrationRepository
{
public:

  bool addBySensor(
    const SensorId& sensorId,
    const Calibration& calibration);

  bool addBySensorType(
    const SensorTypeId& sensorTypeId,
    const Calibration& calibration);

  bool calibrateBySensor(
    const SensorId& sensorId,
    CalibrationKind kind,
    double x,
    double& result) const;

  bool calibrateBySensorType(
    const SensorTypeId& sensorTypeId,
    CalibrationKind kind,
    double x,
    double& result) const;

  [[nodiscard]]
  std::size_t sizeSensors() const noexcept;

  [[nodiscard]]
  std::size_t sizeSensorTypes() const noexcept;

  void clear() noexcept;

private:

  std::unordered_map<
    SensorId,
    std::map<CalibrationKind, Calibration>> m_bySensor;

  std::unordered_map<
    SensorTypeId,
    std::map<CalibrationKind, Calibration>> m_bySensorType;
};

}