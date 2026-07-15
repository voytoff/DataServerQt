#ifndef SUBSCRIBERESPONSE_H
#define SUBSCRIBERESPONSE_H

#include "datatypes.h"

namespace qds
{

/*
 * Порядок проверок
1. tagCount > 0 ?
2. tagCount <= MaxSubscriptionTags ?
3. PublishRate корректный ?
4. Все TagId существуют ?
5. Нет повторяющихся TagId ?
6. Создать Subscription
*/
enum class SubscribeResult : uint8_t
{
  Ok = 0,

  InvalidRate,
  EmptyList,
  TooManyTags,
  DuplicateTag,
  InvalidTag
};

struct SubscribeResponse
{
  SubscribeResult result = SubscribeResult::Ok;
  SubscriptionId id{};
};

}

#endif // SUBSCRIBERESPONSE_H
