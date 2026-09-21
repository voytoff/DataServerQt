#include "datastreamreader.h"

namespace qds
{

bool DataStreamReader::process(
  const DataStreamEvent& event,
  std::vector<DataStreamFrame>& frames)
{
  frames.clear();

  if (const auto* anchor =
      std::get_if<DataStreamAnchor>(&event))
  {
    m_time.startStream(*anchor);
    return true;
  }

  const auto* block =
    std::get_if<DataBlock>(&event);

  if (!block)
    return false;

  if (block->channelCount == 0)
    return false;

  if (block->values.size() !=
      block->channelCount * block->frameCount)
    return false;

  frames.reserve(block->frameCount);

  for (std::size_t i = 0;
       i < block->frameCount;
       ++i)
  {
    DataStreamFrame frame;

    frame.module = block->module;
    frame.frameIndex =
      block->firstFrameIndex + i;

    if (!m_time.timestamp(
          frame.module,
          frame.frameIndex,
          frame.timestamp,
          frame.wallTime))
    {
      frames.clear();
      return false;
    }

    const std::size_t offset =
      i * block->channelCount;

    frame.values.assign(
      block->values.begin() + offset,
      block->values.begin() +
        offset + block->channelCount);

    frames.push_back(
      std::move(frame));
  }

  return true;
}

}