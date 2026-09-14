#pragma once

#include "ilcardmodule.h"
#include "ltr11configuration.h"

#include <memory>
#include <span>

namespace qds
{

class Ltr11Module final : public ILCardModule
{
public:
  explicit Ltr11Module(
    const Ltr11Configuration& configuration);

  ~Ltr11Module() noexcept override;

  bool start() noexcept override;
  void stop() noexcept override;

  [[nodiscard]]
  std::size_t blockFrameCapacity() const noexcept override;

  [[nodiscard]]
  std::size_t readBlock(
    std::span<double> values) noexcept override;

private:
  class Impl;

  std::unique_ptr<Impl> m_impl;
};

} // namespace qds