#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "datatypes.h"
#include "strongid.h"

namespace qds
{

struct FormulaIdTag {};
struct CalibrationIdTag {};

using FormulaId =
  StrongId<FormulaIdTag>;

using CalibrationId =
  StrongId<CalibrationIdTag>;

enum class SignalKind : uint8_t
{
  Raw = 0,
  Calculated,
  Unknown = 0xFF
};

static SignalKind signalKindFromString(
  const std::string& value)
{
  if (value == "Raw")
    return SignalKind::Raw;

  if (value == "Calculated")
    return SignalKind::Calculated;

  return SignalKind::Unknown;
}

enum class SignalMemoryArea : uint8_t
{
  Raw = 0,
  Calculated,
  Unknown = UINT8_MAX
};

struct SignalSource
{
  // используется только для SignalKind::Raw
  TagId tag;
};

enum class CalibrationMode : uint8_t
{
  None = 0,
  BySignal,
  BySignalType
};

constexpr bool isValidCalibrationMode(CalibrationMode mode) noexcept
{
  switch (mode)
  {
  case CalibrationMode::None:
  case CalibrationMode::BySignal:
  case CalibrationMode::BySignalType:
    return true;

  default:
    return false;
  }
}


struct SignalDefinition
{
  SignalId id;
  std::string name;

  SignalKind kind = SignalKind::Raw;
  SignalSource source;

  SensorId sensorId;

  ArchiveRate archiveRate = ArchiveRate::Hz10;
  SignalTypeId signalType;

  // Для Calculated.
  std::string formula;

  // Runtime.
  FormulaId formulaId;
  std::vector<SignalId> dependencies;
};

struct SignalReference
{
  SignalMemoryArea area = SignalMemoryArea::Unknown;
  uint32_t index = 0;

  [[nodiscard]]
  bool isValid() const noexcept
  {
    return area != SignalMemoryArea::Unknown;
  }
};

}