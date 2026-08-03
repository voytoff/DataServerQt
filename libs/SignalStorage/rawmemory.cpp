#include "rawmemory.h"

#include <algorithm>
#include <cassert>

#include "signalmemorylayout.h"

namespace qds
{

void RawMemory::initialize(
  const SignalMemoryLayout& layout) noexcept
{
  m_values.assign(
    layout.rawSignalCount(),
    0.0f);
}


double RawMemory::value(uint32_t index) const
{
  assert(index < m_values.size());

  return m_values[index];
}


void RawMemory::setValue(
  uint32_t index,
  double value)
{
  assert(index < m_values.size());

  m_values[index] = value;
}


void RawMemory::setValues(
  uint32_t firstIndex,
  std::span<const double> values)
{
  assert(firstIndex + values.size() <= m_values.size());

  std::copy(
    values.begin(),
    values.end(),
    m_values.begin() + firstIndex);
}


std::span<const double> RawMemory::values() const noexcept
{
  return m_values;
}


void RawMemory::snapshot(
  std::span<double> destination) const
{
  assert(destination.size() == m_values.size());

  std::copy(
    m_values.begin(),
    m_values.end(),
    destination.begin());
}

}