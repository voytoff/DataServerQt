#pragma once

#include "ilcardmodule.h"
#include <cstdint>

namespace qds
{

class SmartBlockLCardModule final : public qds::ILCardModule
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

  std::size_t readBlock(
    std::span<double> values) noexcept override;

  void setCount(const uint32_t blockCount);

private:
  std::size_t m_frameCount;
  std::size_t m_channelCount;
  uint32_t m_blockCount = 1;
  bool m_running = false;
};

}