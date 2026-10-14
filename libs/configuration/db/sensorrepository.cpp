#include "sensorrepository.h"

namespace qds
{

bool SensorRepository::add(
  const Sensor& sensor)
{
  if (!sensor.id)
    return false;

  return m_sensors.emplace(
    sensor.id, sensor).second;
}

bool SensorRepository::add(
  const SensorType& type)
{
  if (!type.id)
    return false;

  return m_sensorTypes.emplace(
    type.id, type).second;
}

const Sensor* SensorRepository::find(
  SensorId id) const noexcept
{
  const auto it = m_sensors.find(id);

  return it != m_sensors.end()
    ? &it->second
    : nullptr;
}

const SensorType* SensorRepository::find(
  SensorTypeId id) const noexcept
{
  const auto it = m_sensorTypes.find(id);

  return it != m_sensorTypes.end()
    ? &it->second
    : nullptr;
}

void SensorRepository::clear() noexcept
{
  m_sensors.clear();
  m_sensorTypes.clear();
}

std::size_t SensorRepository::sensorCount()
  const noexcept
{
  return m_sensors.size();
}

std::size_t SensorRepository::sensorTypeCount()
  const noexcept
{
  return m_sensorTypes.size();
}

std::vector<Sensor> SensorRepository::sensors() const
{
  std::vector<Sensor> result;
  result.reserve(m_sensors.size());

  for (const auto& [id, sensor] : m_sensors)
    result.push_back(sensor);

  return result;
}

}