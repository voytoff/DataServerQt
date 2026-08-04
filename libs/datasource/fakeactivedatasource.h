#pragma once

#include "iactivedatasource.h"
#include <cstdint>

namespace qds
{

class FakeActiveDataSource : public IActiveDataSource
{
public:
  FakeActiveDataSource() = default;

  bool start() override;
  void stop() noexcept override;

  [[nodiscard]]
  bool step() noexcept override;

  bool isRunning() const noexcept override;

  uint32_t startCalls = 0;
  uint32_t stopCalls = 0;
  uint32_t stepCalls = 0;

private:
  bool m_running = false;

};

}
