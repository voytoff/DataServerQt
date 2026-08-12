#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "datatypes.h"
#include "strongid.h"

namespace qds
{

struct SignalIdTag {};
struct FormulaIdTag {};
struct CalibrationIdTag {};

using SignalId =
  StrongId<SignalIdTag>;

using FormulaId =
  StrongId<FormulaIdTag>;

using CalibrationId =
  StrongId<CalibrationIdTag>;

static constexpr SignalId InvalidSignalId{UINT32_MAX};


enum class SignalKind : uint8_t
{
  Raw = 0,
  Calculated
};

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

  // ссылка на таблицу тарировки
  CalibrationId calibrationId = {0};

  // ссылка на формулу вычисления
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