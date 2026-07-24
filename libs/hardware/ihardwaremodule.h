#ifndef IHARDWAREMODULE_H
#define IHARDWAREMODULE_H

#include <span>

namespace qds
{

class IHardwareModule
{
public:
  virtual ~IHardwareModule() = default;

  virtual bool start() = 0;

  virtual void stop() noexcept = 0;

  virtual bool read(std::span<float> values) = 0;
};

}

#endif
