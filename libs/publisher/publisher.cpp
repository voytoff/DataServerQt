#include "publisher.h"
#include "protocol/publishheader.h"

namespace qds
{

enum class PublishResult
{
  Ok,
  InvalidTag,
  EmptySubscription,
  BufferOverflow
};

bool Publisher::publish(
  const LiveStorage& storage,
  const Subscription& sub,
  uint32_t sequence,
  PacketWriter& writer) const
{
  writer.begin(PacketType::LiveData);

  PublishHeader hdr{};

  hdr.subscriptionId = sub.id;
  hdr.sequence = sequence;
  hdr.valueCount = uint32_t(sub.tags.size());

  if (!sub.tags.empty())
    hdr.timestamp = storage.timestamp(sub.tags.front());

  writer.write(hdr);

  for(TagId tag : sub.tags)
  {
    //if (!storage.contains(tag))
    //{
    //   writer.clear();
    //  return false;
    //}
    writer.write(storage.sample(tag));
  }

  return true;
}

}