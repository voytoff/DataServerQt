#pragma once

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