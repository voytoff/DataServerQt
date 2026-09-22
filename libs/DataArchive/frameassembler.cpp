#include "frameassembler.h"

namespace qds
{

FrameAssembler::FrameAssembler(
  const SystemConfiguration& configuration,
  const SignalMemoryLayout& layout,
  FrameStartPolicy startPolicy)
  : m_layout(layout)
{
  m_frame.initialize(layout);

  m_modules.reserve(
    configuration.modules().size());

  for (const auto& module :
       configuration.modules())
  {
    m_modules.push_back(module.id);
  }
}

std::optional<Frame>
FrameAssembler::push(
  const DataStreamFrame& frame)
{
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
    m_startPolicy == FrameStartPolicy::AllowIncomplete ||
    m_received.size() == m_modules.size();

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
