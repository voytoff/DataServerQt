#include "signalstorage.h"
#include <cassert>

namespace qds
{

void SignalStorage::initialize(const SignalMemoryLayout &layout)
{
  m_raw.assign(layout.rawSignalCount(), 0.0f);
  m_calculated.assign(layout.calculatedSignalCount(), 0.0f);}

float SignalStorage::rawValue(uint32_t index) const
{
  assert(index < m_raw.size());
  return m_raw[index];
}

float SignalStorage::calculatedValue(uint32_t index) const
{
  assert(index < m_calculated.size());
  return m_calculated[index];
}

void SignalStorage::setRawValue(uint32_t index, float value)
{
  assert(index < m_raw.size());
  m_raw[index] = value;
}

void SignalStorage::setCalculatedValue(uint32_t index, float value)
{
  assert(index < m_calculated.size());
  m_calculated[index] = value;
}

}
