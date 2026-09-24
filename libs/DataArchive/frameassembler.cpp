#include "frameassembler.h"

namespace qds
{

FrameAssembler::FrameAssembler(
  const SystemConfiguration& configuration,
  const SignalMemoryLayout& layout,
  FrameStartPolicy startPolicy)
  : m_layout(layout)
  , m_startPolicy(startPolicy)
{
  m_frame.initialize(layout);

  for (const auto& module :
       configuration.modules())
  {
    m_expected.insert(module.id);
  }
}

std::optional<Frame>
FrameAssembler::push(
  const DataStreamFrame& frame)
{
  if (!m_expected.contains(frame.module))
    return std::nullopt;

  const auto offset =
    m_layout.rawOffset(frame.module);

  const auto count =
    m_layout.rawCount(frame.module);

  if (!offset || !count)
    return std::nullopt;

  if (frame.values.size() != *count)
    return std::nullopt;

  m_frame.raw().setValues(
    *offset,
    frame.values);

  m_received.insert(frame.module);

  const bool ready =
    m_startPolicy ==
      FrameStartPolicy::AllowIncomplete ||
    m_received.size() ==
      m_expected.size();

  if (!ready)
    return std::nullopt;

  m_frame.number =
    m_nextFrameNumber;

  m_frame.timestamp =
    frame.timestamp;

  m_frame.wallTime =
    frame.wallTime;

  ++m_nextFrameNumber.value;

  return m_frame;
}

}
