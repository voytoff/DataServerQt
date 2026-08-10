#pragma once

#include <span>

namespace qds
{

class IDataSource
{
public:
  virtual ~IDataSource() noexcept = default;

  [[nodiscard]]
  virtual bool acquire(
    std::span<double> values) = 0;
};

}