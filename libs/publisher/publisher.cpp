#include "publisher.h"
#include "protocol/livedata.h"

namespace qds
{

bool Publisher::publish(const LiveStorage& storage,
                        const Subscription& subscription,
                        uint32_t sequence,
                        uint64_t timestamp,
                        PacketWriter& writer) const
{
  writer.begin(PacketType::LiveData);

  const auto& tags = subscription.tags;

  LiveDataHeader header;

  header.subscriptionId = subscription.id;
  header.sequence       = sequence;
  header.timestamp      = timestamp;
  header.sampleCount =
    static_cast<uint32_t>(tags.size());

  writer.write(header);

  for (TagId tag : tags)
  {
    writer.write(storage.sample(tag).value);
  }

  return true;
}

}