#ifndef ICLOCK_H
#define ICLOCK_H

#include <cstdint>

namespace qds
{

class IClock
{
public:
  virtual ~IClock() = default;

  [[nodiscard]]
  virtual uint64_t now() const noexcept = 0;
};

}

#endif // ICLOCK_H
