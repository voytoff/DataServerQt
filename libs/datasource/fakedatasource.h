#ifndef FAKEDATASOURCE_H
#define FAKEDATASOURCE_H

#include "idatasource.h"
#include <cstdint>

namespace qds
{

class FakeDataSource : public IDataSource
{
public:
  FakeDataSource() = default;

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

#endif // FAKEDATASOURCE_H
