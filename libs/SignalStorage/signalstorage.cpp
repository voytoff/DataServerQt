#include <cassert>
#include <span>
#include "signalstorage.h"
#include "signalmemorylayout.h"

namespace qds
{

void SignalStorage::initialize(const SignalMemoryLayout& layout)
{
  m_raw.assign(layout.rawSignalCount(), 0.0f);
  m_calculated.assign(layout.calculatedSignalCount(), 0.0f);
}

double SignalStorage::rawValue(uint32_t index) const
{
  assert(index < m_raw.size());
  return m_raw[index];
}

double SignalStorage::calculatedValue(uint32_t index) const
{
  assert(index < m_calculated.size());
  return m_calculated[index];
}

void SignalStorage::setRawValue(uint32_t index, double value)
{
  assert(index < m_raw.size());
  m_raw[index] = value;
}

void SignalStorage::setCalculatedValue(uint32_t index, double value)
{
  assert(index < m_calculated.size());
  m_calculated[index] = value;
}

std::span<const double> SignalStorage::rawValues() const noexcept
{
  return m_raw;
}

std::span<const double> SignalStorage::calculatedValues() const noexcept
{
  return m_calculated;
}

std::span<double> SignalStorage::rawValues() noexcept
{
  return m_raw;
}

std::span<double> SignalStorage::calculatedValues() noexcept
{
  return m_calculated;
}

}
