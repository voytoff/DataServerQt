#pragma once

#include <strongidhash.h>
#include <cstdint>
#include <unordered_map>

#include "signaldefinition.h"
#include "systemconfiguration.h"

namespace qds
{

struct SignalLocation
{
  SignalId id;

  SignalMemoryArea area =
    SignalMemoryArea::Raw;

  uint32_t index = 0;
};


class SignalMemoryLayout
{
public:

  void build(
    const SystemConfiguration& configuration);

  [[nodiscard]]
  bool contains(
    SignalId id) const noexcept;

  [[nodiscard]]
  SignalReference reference(
    SignalId id) const;

  [[nodiscard]]
  uint32_t rawSignalCount() const noexcept;

  [[nodiscard]]
  uint32_t calculatedSignalCount() const noexcept;

  [[nodiscard]]
  uint32_t rawOffset(ModuleId module) const;

  const SignalLocation &location(SignalId id) const;

private:

  std::unordered_map<
    SignalId,
    SignalLocation> m_locations;

  std::unordered_map<
    ModuleId,
    uint32_t> m_rawOffsets;

  uint32_t m_rawSignalCount = 0;

  uint32_t m_calculatedSignalCount = 0;
};

}