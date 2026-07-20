#include "publisher.h"
#include "protocol/publishheader.h"

namespace qds
{

bool Publisher::publish(const LiveStorage& storage,
                        const Subscription& sub,
                        uint32_t sequence,
                        uint64_t timestamp,
                        PacketWriter& writer) const
{
  writer.begin(PacketType::LiveData);

  const auto& tags = sub.tags;

  PublishHeader hdr;
  hdr.subscriptionId = sub.id;
  hdr.sequence = sequence;
  hdr.timestamp = timestamp;
  hdr.valueCount = uint32_t(tags.size());

  writer.write(hdr);

  for (TagId tag : tags)
  {
    Sample sample;

    if (!storage.read(tag, sample)) {
      writer.clear();
      return false;
    }

    writer.write(sample);
  }

  return true;
}

}