#include "calibrationrepository.h"

namespace qds
{

bool CalibrationRepository::addBySensor(
  const SensorId& sensorId,
  const Calibration& calibration)
{
  if (sensorId.value == 0)
    return false;

  if (calibration.sensorId != sensorId ||
      calibration.sensorTypeId.value != 0)
    return false;

  auto& calibrations = m_bySensor[sensorId];

  auto [it, inserted] =
    calibrations.emplace(
      calibration.kind,
      calibration);

  return inserted;
}

bool CalibrationRepository::addBySensorType(
  const SensorTypeId& sensorTypeId,
  const Calibration& calibration)
{
  if (sensorTypeId.value == 0)
    return false;

  if (calibration.sensorTypeId != sensorTypeId ||
      calibration.sensorId.value != 0)
    return false;

  auto& calibrations = m_bySensorType[sensorTypeId];

  auto [it, inserted] =
    calibrations.emplace(
      calibration.kind,
      calibration);

  return inserted;
}

bool CalibrationRepository::calibrateBySensor(
  const SensorId& sensorId,
  CalibrationKind kind,
  double x,
  double& result) const
{
  auto it = m_bySensor.find(sensorId);

  if (it == m_bySensor.end())
    return false;

  auto calibrationIt = it->second.find(kind);

  if (calibrationIt == it->second.end())
    return false;

  return calibrationIt->second.apply(x, result);
}

bool CalibrationRepository::calibrateBySensorType(
  const SensorTypeId& sensorTypeId,
  CalibrationKind kind,
  double x,
  double& result) const
{
  auto it = m_bySensorType.find(sensorTypeId);

  if (it == m_bySensorType.end())
    return false;

  auto calibrationIt = it->second.find(kind);

  if (calibrationIt == it->second.end())
    return false;

  return calibrationIt->second.apply(x, result);
}

std::size_t CalibrationRepository::sizeSensors() const noexcept
{
  return m_bySensor.size();
}

std::size_t CalibrationRepository::sizeSensorTypes() const noexcept
{
  return m_bySensorType.size();
}

void CalibrationRepository::clear() noexcept
{
  m_bySensor.clear();
  m_bySensorType.clear();
}

const Calibration* CalibrationRepository::findBySensor(
  SensorId sensorId,
  CalibrationKind kind) const noexcept
{
  const auto it = m_bySensor.find(sensorId);

  if (it == m_bySensor.end())
    return nullptr;

  const auto calibrationIt = it->second.find(kind);

  return calibrationIt != it->second.end()
           ? &calibrationIt->second
           : nullptr;
}

const Calibration* CalibrationRepository::findBySensorType(
  SensorTypeId sensorTypeId,
  CalibrationKind kind) const noexcept
{
  const auto it = m_bySensorType.find(sensorTypeId);

  if (it == m_bySensorType.end())
    return nullptr;

  const auto calibrationIt = it->second.find(kind);

  return calibrationIt != it->second.end()
           ? &calibrationIt->second
           : nullptr;
}

}