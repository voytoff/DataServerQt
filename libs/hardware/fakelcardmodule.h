#ifndef FAKELCARDMODULE_H
#define FAKELCARDMODULE_H

#include "imoduledevice.h"
#include <cstdint>

namespace qds
{

class FakeLCardModule : public IModuleDevice
{
public:
  bool start() override;
  void stop() noexcept override;

  bool read(std::span<float> values) override;

public:
  uint32_t startCalls = 0;
  uint32_t stopCalls = 0;
  uint32_t readCalls = 0;


private:
  bool m_running = false;
  float counter = 0;
};

}

#endif // FAKELCARDMODULE_H
