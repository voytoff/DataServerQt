#include "frameaccessor.h"
#include "signaldefinition.h"

namespace qds
{

double FrameAccessor::read(
  const Frame& frame,
  SignalReference reference) noexcept
{
  switch (reference.area)
  {
  case SignalMemoryArea::Raw:
    return frame.raw.value(reference.index);

  case SignalMemoryArea::Calculated:
    return frame.calculated.value(reference.index);
  }

  return 0.0;
}


void FrameAccessor::write(
  Frame& frame,
  SignalReference reference,
  double value) noexcept
{
  switch (reference.area)
  {
  case SignalMemoryArea::Raw:
    frame.raw.setValue(
      reference.index,
      value);
    break;

  case SignalMemoryArea::Calculated:
    frame.calculated.setValue(
      reference.index,
      value);
    break;
  }
}

}