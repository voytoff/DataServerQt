#include "signalmemorylayout.h"
#include "signaldefinition.h"
#include "systemconfiguration.h"
#include "signalmemorylayout.h"

#include <algorithm>
#include <utility>
#include <strongidhash.h>
#include <cassert>
#include <unordered_map>

namespace qds
{

bool SignalMemoryLayout::build(
  const SystemConfiguration& configuration)
{
  SignalMemoryLayout tmp;

  const auto& definitions =
    configuration.signalDefinitions();

  auto fail =
    [this](std::string message)
  {
    m_lastError = std::move(message);
    return false;
  };

  // 1. Проверяем уникальность SignalId
  // и допустимость SignalKind.

  std::unordered_map<TagId, const SignalDefinition*> rawSignals;
  std::unordered_map<SignalId, bool> signalIds;

  for (const auto& definition : definitions)
  {
    if (!signalIds.emplace(definition.id, true).second)
    {
      return fail(
        "Повторяется идентификатор сигнала (SignalId): " +
        std::to_string(definition.id.value));
    }

    switch (definition.kind)
    {
    case SignalKind::Raw:
    {
      if (!rawSignals.emplace(
                       definition.source.tag,
                       &definition).second)
      {
        return fail(
          "Несколько Raw-сигналов используют один источник (TagId): " +
          std::to_string(definition.source.tag.value));
      }

      break;
    }

    case SignalKind::Calculated:
      break;

    default:
      return fail(
        "Неизвестный вид сигнала (SignalId): " +
        std::to_string(definition.id.value));
    }
  }

  // 2. Строим RawMemory.

  uint32_t rawOffset = 0;

  for (const auto& module : configuration.modules())
  {
    if (!tmp.m_rawOffsets.emplace(
                           module.id,
                           rawOffset).second)
    {
      return fail(
        "Повторяется идентификатор модуля (ModuleId): " +
        std::to_string(module.id.value));
    }

    uint32_t rawCount = 0;

    for (const TagId& tag :
         configuration.moduleTags(module.id))
    {
      const auto it = rawSignals.find(tag);

      if (it == rawSignals.end())
      {
        return fail(
          "Не найден Raw-сигнал для модуля (ModuleId): " +
          std::to_string(module.id.value) +
          ", источника (TagId): " +
          std::to_string(tag.value));
      }

      const SignalDefinition& definition =
        *it->second;

      SignalLocation location;

      location.id = definition.id;
      location.area = SignalMemoryArea::Raw;
      location.index = tmp.m_rawSignalCount;

      if (!tmp.m_locations.emplace(
                            definition.id,
                            location).second)
      {
        return fail(
          "Повторное размещение Raw-сигнала в памяти (SignalId): " +
          std::to_string(definition.id.value));
      }

      ++tmp.m_rawSignalCount;
      ++rawOffset;
      ++rawCount;
    }

    tmp.m_rawCounts.emplace(
      module.id,
      rawCount);
  }

  // 3. Строим CalculatedMemory.

  for (const auto& definition : definitions)
  {
    if (definition.kind != SignalKind::Calculated)
      continue;

    SignalLocation location;

    location.id = definition.id;
    location.area = SignalMemoryArea::Calculated;
    location.index = tmp.m_calculatedSignalCount;

    if (!tmp.m_locations.emplace(
                          definition.id,
                          location).second)
    {
      return fail(
        "Повторное размещение Calculated-сигнала в памяти (SignalId): " +
        std::to_string(definition.id.value));
    }

    ++tmp.m_calculatedSignalCount;
  }

  // 4. Проверяем, что каждый Raw-сигнал
  // действительно попал в layout.

  for (const auto& definition : definitions)
  {
    if (definition.kind != SignalKind::Raw)
      continue;

    if (!tmp.m_locations.contains(definition.id))
    {
      return fail(
        "Raw-сигнал не связан с активным источником (SignalId): " +
        std::to_string(definition.id.value));
    }
  }

  // 5. Публикуем результат только после
  // полного успешного построения.

  m_locations = std::move(tmp.m_locations);
  m_rawOffsets = std::move(tmp.m_rawOffsets);
  m_rawCounts = std::move(tmp.m_rawCounts);

  m_rawSignalCount = tmp.m_rawSignalCount;
  m_calculatedSignalCount = tmp.m_calculatedSignalCount;

  m_lastError.clear();

  return true;
}

const std::string&
SignalMemoryLayout::lastError() const noexcept
{
  return m_lastError;
}


uint32_t SignalMemoryLayout::rawSignalCount() const noexcept
{
  return m_rawSignalCount;
}

uint32_t SignalMemoryLayout::calculatedSignalCount() const noexcept
{
  return m_calculatedSignalCount;
}

std::optional<uint32_t> SignalMemoryLayout::rawOffset(ModuleId module) const
{
  auto it = m_rawOffsets.find(module);

  if (it == m_rawOffsets.end())
    return std::nullopt;

  return it->second;
}

const SignalLocation &SignalMemoryLayout::location(SignalId id) const
{
  auto it = m_locations.find(id);

  assert(it != m_locations.end());

  return it->second;
}

std::optional<uint32_t>
SignalMemoryLayout::rawCount(
  ModuleId module) const
{
  const auto it =
    m_rawCounts.find(module);

  if (it == m_rawCounts.end())
    return std::nullopt;

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

  if (it == m_locations.end())
    return {};

  return {
    .area = it->second.area,
    .index = it->second.index
  };
}

}
