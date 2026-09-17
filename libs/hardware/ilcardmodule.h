#pragma once

#include <cstddef>
#include <span>

namespace qds
{

class ILCardModule
{
public:
  virtual ~ILCardModule() noexcept = default;

  virtual bool start() noexcept = 0;
  virtual void stop() noexcept = 0;

  [[nodiscard]]
  virtual std::size_t blockFrameCapacity() const noexcept = 0;

  [[nodiscard]]
  virtual double frameRate() const noexcept = 0;

  [[nodiscard]]
  virtual std::size_t readBlock(
    std::span<double> values) noexcept = 0;
};

}