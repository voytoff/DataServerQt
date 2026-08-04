#include <cassert>
#include <vector>

#include "signalmemorylayout.h"
#include "signaldefinition.h"
#include "systemconfiguration.h"

namespace qds
{

void SignalMemoryLayout::build(const SystemConfiguration &configuration)
{
  m_locations.clear();

  m_rawSignalCount = 0;
  m_calculatedSignalCount = 0;

  uint32_t rawIndex = 0;
  uint32_t calculatedIndex = 0;

  SignalId maxId = {0};

  const auto& definitions = configuration.signalDefinitions();

  for (const auto& definition : definitions)
    maxId = std::max(maxId, definition.id);

  m_locations.assign(maxId.value + 1, {});

  for (const auto& definition : definitions)
  {
    auto id = definition.id.value;
    assert(id < m_locations.size());

    switch (definition.kind)
    {
    case SignalKind::Raw:
      m_locations[id].area = SignalMemoryArea::Raw;
      m_locations[id].index = rawIndex++;
      break;

    case SignalKind::Calculated:
      m_locations[id].area = SignalMemoryArea::Calculated;
      m_locations[id].index = calculatedIndex++;
      break;

    default:
      assert(false);
    }
  }

#ifndef NDEBUG

  std::printf("\nRAW\n");
  std::printf("%-8s %-8s %s\n", "Index", "Id", "Name");
  std::printf("------------------------------------------\n");
  for (const auto& definition : definitions)
  {
    const auto& location = m_locations[definition.id.value];

    if (location.area != SignalMemoryArea::Raw)
      continue;

    std::printf("%-8u %-8u %s\n",
                location.index,
                definition.id.value,
                definition.name.c_str());
  }

  std::printf("\nCALCULATED\n");
  std::printf("%-8s %-8s %s\n", "Index", "Id", "Name");
  std::printf("------------------------------------------\n");
  for (const auto& definition : definitions)
  {
    const auto& location = m_locations[definition.id.value];

    if (location.area != SignalMemoryArea::Calculated)
      continue;

    std::printf("%-8u %-8u %s\n",
                location.index,
                definition.id.value,
                definition.name.c_str());
  }

#endif

  m_rawSignalCount = rawIndex;
  m_calculatedSignalCount = calculatedIndex;
}

const SignalLocation &SignalMemoryLayout::location(SignalId id) const
{
  assert(id.value < m_locations.size());
  return m_locations[id.value];
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
  return id.value < m_locations.size() &&
         m_locations[id.value].index != InvalidIndex32;
}

}
