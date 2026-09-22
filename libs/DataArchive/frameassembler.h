#pragma once

#include "datastreamframe.h"
#include "frame.h"
#include "framestartpolicy.h"
#include "signalmemorylayout.h"
#include "systemconfiguration.h"

#include <optional>
#include <unordered_set>
#include <vector>

namespace qds
{

class FrameAssembler
{
public:
  FrameAssembler(
    const SystemConfiguration& configuration,
    const SignalMemoryLayout& layout,
    FrameStartPolicy startPolicy =
    FrameStartPolicy::WaitForAllModules);

  std::optional<Frame> push(
    const DataStreamFrame& frame);

private:
  const SignalMemoryLayout& m_layout;
  std::vector<ModuleId> m_modules;
  std::unordered_set<ModuleId> m_received;

  Frame m_frame;
  FrameNumber m_nextFrameNumber{0};

  FrameStartPolicy m_startPolicy;
};

}