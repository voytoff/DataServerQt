#ifndef SIGNALMEMORYLAYOUT_H
#define SIGNALMEMORYLAYOUT_H

#include <cstdint>
#include <vector>

#include "datatypes.h"
#include "signaldefinition.h"

namespace qds
{

struct SignalLocation
{
  SignalMemoryArea area = SignalMemoryArea::Raw;
  uint32_t index = InvalidIndex32;
};

class SystemConfiguration;

class SignalMemoryLayout
{
public:

  void build(const SystemConfiguration& configuration);

  const SignalLocation& location(SignalId id) const;

  uint32_t rawSignalCount() const;

  uint32_t calculatedSignalCount() const;

  bool contains(SignalId id) const;

private:

  std::vector<SignalLocation> m_locations;
  uint32_t m_rawSignalCount = 0;
  uint32_t m_calculatedSignalCount = 0;
};

}

#endif // SIGNALMEMORYLAYOUT_H
