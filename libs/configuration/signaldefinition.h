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

struct SignalDefinition
{
  // постоянный идентификатор сигнала
  SignalId id = {0};

  // имя, используемое пользователем
  std::string name;

  // физический или вычисляемый сигнал
  SignalKind kind = SignalKind::Raw;

  // источник для RAW сигнала
  // для Calculated может быть пустым
  SignalSource source;

  // частота записи в архив
  uint32_t archiveFrequency = 0;

  // источник таблицы тарировки
  CalibrationMode calibrationMode = CalibrationMode::None;

  // тип датчика
  SignalTypeId signalType;

  // текст формулы
  std::string formula;

  // Runtime
  // ссылка на формулу вычисления в репозитории
  FormulaId formulaId = {0};
  // аргументы формулы
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