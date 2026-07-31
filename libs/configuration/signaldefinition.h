#ifndef SIGNALDEFINITION_H
#define SIGNALDEFINITION_H

#include <cstdint>
#include <string>

#include "datatypes.h"

namespace qds
{

using SignalId = uint32_t;

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
  // источник RAW сигнала
  TagId tag;
};

struct SignalDefinition
{
  // постоянный идентификатор сигнала
  SignalId id = 0;

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
  uint32_t calibrationId = 0;

  // ссылка на формулу вычисления
  uint32_t formulaId = 0;
};

}

#endif // SIGNALDEFINITION_H