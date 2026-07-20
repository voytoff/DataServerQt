#include "publisher.h"
#include "protocol/publishheader.h"

namespace qds
{

bool Publisher::publish(const LiveStorage& storage,
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
  {
    Sample dummy;

    if (!storage.read(sub.tags.front(), dummy))
    {
      writer.clear();
      return false;
    }

    hdr.timestamp = storage.timestamp(sub.tags.front());
  }

  writer.write(hdr);

  for (TagId tag : sub.tags)
  {
    Sample sample;

    if (!storage.read(tag, sample))
    {
      writer.clear();
      return false;
    }

    writer.write(sample);
  }

  return true;
}

}