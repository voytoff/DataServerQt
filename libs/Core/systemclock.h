#ifndef SYSTEMCLOCK_H
#define SYSTEMCLOCK_H

#include "iclock.h"

namespace qds
{

class SystemClock final : public IClock
{
public:
  [[nodiscard]]
  uint64_t now() const noexcept override;
};

}

#endif // SYSTEMCLOCK_H
