#pragma once

#include "ilcardmodule.h"

#include <cstdint>

namespace qds
{

class FakeLCardModule : public ILCardModule
{
public:
  FakeLCardModule(
    std::size_t channelCount,
    std::size_t blockFrameCount = 10,
    double frameRate = 1000.0);

  bool start() noexcept override;
  void stop() noexcept override;

  std::size_t blockFrameCapacity() const noexcept override;

  double frameRate() const noexcept override;

  std::size_t readBlock(
    std::span<double> values) noexcept override;

public:
  uint32_t startCalls = 0;
  uint32_t stopCalls = 0;
  uint32_t readCalls = 0;

private:
  std::size_t m_channelCount = 0;
  std::size_t m_blockFrameCount = 0;
  double m_frameRate = 0.0;

  bool m_running = false;
  double m_counter = 0.0;
};

}