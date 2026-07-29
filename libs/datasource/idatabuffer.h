#ifndef IDATABUFFER_H
#define IDATABUFFER_H

#include <span>
#include "datatypes.h"

namespace qds
{

class IDataBuffer
{
public:
  virtual ~IDataBuffer() = default;

  virtual ModuleId module() const noexcept = 0;

  virtual uint64_t timestamp() const noexcept = 0;

  virtual std::span<const float> values() const noexcept = 0;
};

}

#endif // IDATABUFFER_H
