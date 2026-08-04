#pragma once

#include <compare>
#include <cstdint>
#include <string>
#include <vector>

#include "datatypes.h"

namespace qds
{

struct SignalId
{
  uint32_t value = 0;
  constexpr auto operator<=>(const SignalId&) const = default;
};

struct FormulaId
{
  uint32_t value = 0;
  constexpr auto operator<=>(const FormulaId&) const = default;
};

struct CalibrationId
{
  uint32_t value = 0;
  constexpr auto operator<=>(const CalibrationId&) const = default;
};

enum class SignalKind : uint8_t
{
  Raw = 0,
  Calculated
};

enum class SignalMemoryArea : uint8_t
{
  Raw = 0,
  Calculated
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
  SignalMemoryArea area;
  uint32_t index;
};

}