#pragma once

#include <cstdint>

#include "datatypes.h"

namespace qds
{

struct PublishHeader
{
  // Подписка, для которой сформирован пакет.
  SubscriptionId subscriptionId;

  // Номер пакета в рамках подписки.
  // Первый успешно отправленный пакет имеет sequence == 1.
  uint32_t sequence;

  // Время измерений Frame.
  uint64_t timestamp;

  // Количество SignalId/значений в пакете.
  uint32_t valueCount;
};

}