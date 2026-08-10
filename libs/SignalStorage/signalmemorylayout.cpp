#include <strongidhash.h>
#include <cassert>
#include <unordered_map>

#include "signalmemorylayout.h"
#include "signaldefinition.h"
#include "systemconfiguration.h"

namespace qds
{

void SignalMemoryLayout::build(
  const SystemConfiguration& configuration)
{
  m_locations.clear();

  m_rawSignalCount = 0;
  m_calculatedSignalCount = 0;

  m_rawOffsets.clear();

  const auto &definitions = configuration.signalDefinitions();

  // 1. заполняем RawMemory
  uint32_t rawOffset = 0;

  for (const auto& module : configuration.modules())
  {
    auto [it, inserted] =
      m_rawOffsets.emplace(
        module.id,
        rawOffset);

    assert(inserted);

    for (const TagId& tag :
         configuration.moduleTags(module.id))
    {
      auto p =
        std::find_if(
          definitions.begin(),
          definitions.end(),
          [&](const SignalDefinition& def)
          {
            return def.kind == SignalKind::Raw &&
                   def.source.tag == tag;
          });

      assert(p != definitions.end());

      SignalLocation location;

      location.id = p->id;
      location.area = SignalMemoryArea::Raw;
      location.index = m_rawSignalCount++;

      auto [it, inserted] =
        m_locations.emplace(
          p->id,
          location);

      assert(inserted);

      ++rawOffset;
    }
  }

  assert(rawOffset == m_rawSignalCount);

  // 2. заполняем CalculatedMemory
  for (const auto& definition : definitions)
  {
    SignalLocation location;

    location.id = definition.id;

    switch(definition.kind)
    {
    case SignalKind::Raw:

      continue;

    case SignalKind::Calculated:

      location.area =
        SignalMemoryArea::Calculated;

      location.index =
        m_calculatedSignalCount++;

      break;


    default:
      assert(false);
    }

    auto [it, inserted] =
      m_locations.emplace(
        definition.id,
        location);

    assert(inserted);
  }

  assert(rawOffset == m_rawSignalCount);

#ifndef NDEBUG

  std::printf("\nRAW\n");
  std::printf("%-8s %-8s %s\n", "Index", "Id", "Name");
  std::printf("------------------------------------------\n");
  for (const auto& definition : definitions)
  {
    const auto& location = m_locations.at(definition.id);

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
    const auto& location = m_locations.at(definition.id);

    if (location.area != SignalMemoryArea::Calculated)
      continue;

    std::printf("%-8u %-8u %s\n",
                location.index,
                definition.id.value,
                definition.name.c_str());
  }

#endif

}

uint32_t SignalMemoryLayout::rawSignalCount() const noexcept
{
  return m_rawSignalCount;
}

uint32_t SignalMemoryLayout::calculatedSignalCount() const noexcept
{
  return m_calculatedSignalCount;
}

uint32_t SignalMemoryLayout::rawOffset(ModuleId module) const
{
  auto it = m_rawOffsets.find(module);

  if (it == m_rawOffsets.end())
    return 0;

  return it->second;
}

const SignalLocation &SignalMemoryLayout::location(SignalId id) const
{
  auto it = m_locations.find(id);

  assert(it != m_locations.end());

  return it->second;
}

bool SignalMemoryLayout::contains(
  SignalId id) const noexcept
{
  return m_locations.contains(id);
}

SignalReference SignalMemoryLayout::reference(
  SignalId id) const
{
  auto it =
    m_locations.find(id);
  // проверка ошибки конфигурации
  assert(it != m_locations.end());

  return {
    .area = it->second.area,
    .index = it->second.index
  };
}

}
