#include "frame.h"
#include <cassert>

namespace qds
{

void Frame::initialize(const SignalMemoryLayout& layout)
{
  m_raw.initialize(layout.rawSignalCount());
  m_calculated.initialize(layout.calculatedSignalCount());
}

double* Frame::address(
  SignalReference ref) noexcept
{
  switch(ref.area)
  {
  case SignalMemoryArea::Raw:
    assert(ref.index < m_raw.size());
    return &m_raw.valueRef(ref.index);

  case SignalMemoryArea::Calculated:
    assert(ref.index < m_calculated.size());
    return &m_calculated.valueRef(ref.index);
  }

  assert(false);
  return nullptr;
}

const double *Frame::address(SignalReference ref) const noexcept
{
  switch(ref.area)
  {
  case SignalMemoryArea::Raw:
    assert(ref.index < m_raw.size());
    return &m_raw.valueRef(ref.index);

  case SignalMemoryArea::Calculated:
    assert(ref.index < m_calculated.size());
    return &m_calculated.valueRef(ref.index);
  }

  assert(false);
  return nullptr;
}

RawMemory& Frame::raw() noexcept
{
  return m_raw;
}

const RawMemory& Frame::raw() const noexcept
{
  return m_raw;
}


CalculatedMemory& Frame::calculated() noexcept
{
  return m_calculated;
}

const CalculatedMemory& Frame::calculated() const noexcept
{
  return m_calculated;
}

}
