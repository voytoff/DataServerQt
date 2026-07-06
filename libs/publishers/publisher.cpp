#include "publisher.h"
#include "protocol/livedata.h"
#include <cstdint>

namespace qds
{

bool Publisher::publish(const Subscription &subscription, uint32_t sequence, uint64_t timestamp, PacketWriter &writer) const
{
  LiveDataHeader header;

  header.subscriptionId = subscription.id;
  header.sequence = sequence;
  header.timestamp = timestamp;
  header.sampleCount = subscription.tags.size();

  writer.begin(PacketType::LiveData);

  writer.write(header);

  for (TagId tag : subscription.tags)
  {
    float value = m_storage.sample(tag).value;

    writer.write(value);
  }
}

}