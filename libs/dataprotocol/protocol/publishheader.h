#ifndef PUBLISHHEADER_H
#define PUBLISHHEADER_H

#include <cstdint>
#include "datatypes.h"

namespace qds
{

struct PublishHeader    // 20 байт
{
  SubscriptionId subscriptionId; // подписка
  uint32_t sequence;    // номер пакета для данной подписки
                        // sequence принадлежит подписке
                        // начинается с 1
                        // увеличивается только после успешной отправки
  uint64_t timestamp;   // реальное время измерений
  uint32_t valueCount;  // количество значений далее (tags.size())
};

}

#endif // PUBLISHHEADER_H
