#include <cassert>
#include "signalmemorylayout.h"
#include "signaldefinition.h"
#include "systemconfiguration.h"

namespace  qds {

void SignalMemoryLayout::build(const SystemConfiguration &configuration)
{
  m_locations.clear();

  m_rawSignalCount = 0;
  m_calculatedSignalCount = 0;

  uint32_t rawIndex = 0;
  uint32_t calculatedIndex = 0;

  auto signalDefinitions = configuration.signalDefinitions();
  for(int i = 0; i < signalDefinitions.size(); i++)
  {
    SignalDefinition definition = signalDefinitions[i];

    if (definition.kind == SignalMemoryArea::Raw)
    {
      m_locations.push_back({.area = SignalMemoryArea::Raw, .index = rawIndex++});
    }
    else if (definition.kind == SignalMemoryArea::Calculated)
    {
      m_locations.push_back({.area = SignalMemoryArea::Calculated, .index = calculatedIndex++});
    }
  }

  m_locations.resize(rawIndex + calculatedIndex);

  m_rawSignalCount = rawIndex;
  m_calculatedSignalCount = calculatedIndex;
}

const SignalLocation &SignalMemoryLayout::location(SignalId id) const
{
  assert(id < m_locations.size());
  return m_locations[id];
}

uint32_t SignalMemoryLayout::rawSignalCount() const
{
  return m_rawSignalCount;
}

uint32_t SignalMemoryLayout::calculatedSignalCount() const
{
  return m_calculatedSignalCount;
}

bool SignalMemoryLayout::contains(SignalId id) const
{
  return id < m_locations.size() && m_locations[id].index != InvalidIndex;
}

}
