#pragma once

#include "ilcardmodule.h"
#include <atomic>
#include <cstdint>

namespace qds
{

class SmartBlockLCardModule final : public ILCardModule
{
public:
  SmartBlockLCardModule(
    const std::size_t frameCount,
    const std::size_t channelCount,
    const uint32_t blockCount = 1
    );

  bool start() noexcept override;

  void stop() noexcept override;

  std::size_t blockFrameCapacity() const noexcept override;

  double frameRate() const noexcept override;

  [[nodiscard]]
  uint32_t remainingBlockCount() const noexcept;

  std::size_t readBlock(
    std::span<double> values) noexcept override;

  void setCount(const uint32_t blockCount);

private:
  std::size_t m_frameCount;
  std::size_t m_channelCount;
  std::atomic_uint32_t m_blockCount{0};
  std::atomic_bool m_running{false};
};

}