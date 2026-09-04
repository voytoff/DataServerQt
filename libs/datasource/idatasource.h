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

  virtual bool start() noexcept = 0;

  virtual void stop() noexcept = 0;

};

}