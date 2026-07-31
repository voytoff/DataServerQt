#ifndef SIGNALAREA_H
#define SIGNALAREA_H

#include <cstdint>

namespace qds
{

enum class SignalMemoryArea : uint8_t
{
  Raw,
  Calculated
};

}

#endif // SIGNALAREA_H
