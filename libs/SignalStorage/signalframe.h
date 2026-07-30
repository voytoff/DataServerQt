#ifndef SIGNALFRAME_H
#define SIGNALFRAME_H

#include <span>
#include "signalstorage.h"

namespace qds
{

struct SignalFrame
{
  uint64_t timestamp = 0;

  std::span<const SignalId> ids;
  std::span<const double> values;
};

}

#endif // SIGNALFRAME_H
