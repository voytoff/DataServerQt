#pragma once

#include "sensor.h"
#include "sensortype.h"

#include <strongidhash.h>
#include <unordered_map>
#include <vector>

namespace qds
{

/** датчики и их типы. */
class SensorRepository
{
public:

  bool add(const Sensor& sensor);
  bool add(const SensorType& type);

  [[nodiscard]]
  const Sensor* find(SensorId id) const noexcept;

  [[nodiscard]]
  const SensorType* find(
    SensorTypeId id) const noexcept;

  void clear() noexcept;

  [[nodiscard]]
  std::size_t sensorCount() const noexcept;

  [[nodiscard]]
  std::size_t sensorTypeCount() const noexcept;

  [[nodiscard]]
  std::vector<Sensor> sensors() const;

  [[nodiscard]]
  std::vector<SensorType> sensorTypes() const;

private:

  std::unordered_map<SensorId, Sensor> m_sensors;

  std::unordered_map<SensorTypeId, SensorType>
    m_sensorTypes;
};

}