#pragma once

#include "ilcardmodule.h"

#include <cstdint>

namespace qds
{

class FakeLCardModule : public ILCardModule
{
public:
  bool start() noexcept override;
  void stop() noexcept override;

  bool read(std::span<double> values) noexcept override;

public:
  uint32_t startCalls = 0;
  uint32_t stopCalls = 0;
  uint32_t readCalls = 0;

private:
  bool m_running = false;
  double m_counter = 0.0;
};

}