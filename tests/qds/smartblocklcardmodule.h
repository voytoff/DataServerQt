#pragma once

#include "ilcardmodule.h"
#include <atomic>
#include <cstdint>
#include <optional>

namespace qds
{

class SmartBlockLCardModule final : public ILCardModule
{
public:
  SmartBlockLCardModule(
    const std::size_t frameCount,
    const std::size_t channelCount,
    const uint32_t blockCount = 1,
    const std::optional<uint32_t> successCount = std::nullopt
    );

  bool start() noexcept override;

  void stop() noexcept override;

  std::size_t blockFrameCapacity() const noexcept override;

  double frameRate() const noexcept override;

  [[nodiscard]]
  uint32_t remainingBlockCount() const noexcept;

  LCardReadResult readBlock(
    std::span<double> values) noexcept override;

  void setBlockCount(const uint32_t blockCount);
  void setSuccessCount(const std::optional<uint32_t> successCount);

public:
  uint32_t stopCalls = 0;

private:
  std::size_t m_frameCount;
  std::size_t m_channelCount;
  std::atomic<std::optional<uint32_t>> m_successCount;
  std::atomic_uint32_t m_blockCount{0};
  std::atomic_bool m_running{false};
};

}