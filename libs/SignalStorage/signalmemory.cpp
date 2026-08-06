#include <algorithm>
#include <cassert>

#include "signalmemory.h"

namespace qds
{

void SignalMemory::initialize(uint32_t count) noexcept
{
  m_values.assign(count, 0.0);
}

double SignalMemory::value(uint32_t index) const
{
  assert(index < m_values.size());

  return m_values[index];
}


void SignalMemory::setValue(
  uint32_t index,
  double value)
{
  assert(index < m_values.size());

  m_values[index] = value;
}


void SignalMemory::setValues(
  uint32_t firstIndex,
  std::span<const double> values)
{
  assert(firstIndex + values.size() <= m_values.size());

  std::copy(
    values.begin(),
    values.end(),
    m_values.begin() + firstIndex);
}

double& SignalMemory::valueRef(uint32_t index) noexcept
{
  assert(index < m_values.size());
  return m_values[index];
}

const double& SignalMemory::valueRef(uint32_t index) const noexcept
{
  assert(index < m_values.size());
  return m_values[index];
}

std::span<const double> SignalMemory::values() const noexcept
{
  return m_values;
}

std::span<double> SignalMemory::values() noexcept
{
  return m_values;
}


void SignalMemory::snapshot(std::span<double> destination) const
{
  assert(destination.size() == m_values.size());

  std::copy(
    m_values.begin(),
    m_values.end(),
    destination.begin());
}

void SignalMemory::restore(std::span<const double> source)
{
  assert(source.size() == m_values.size());

  std::copy(
    source.begin(),
    source.end(),
    m_values.begin());
}

bool SignalMemory::equals(std::span<const double> snapshot) const
{
  assert(snapshot.size() == m_values.size());

  return std::equal(
    snapshot.begin(),
    snapshot.end(),
    m_values.begin());
}

void SignalMemory::clear() noexcept
{
  std::fill(m_values.begin(), m_values.end(), 0.0);
}

uint32_t SignalMemory::size() const noexcept
{
  return m_values.size();
}

}