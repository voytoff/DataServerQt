#ifndef IMODULEDEVICE_H
#define IMODULEDEVICE_H

#include <span>

namespace qds
{

class IModuleDevice
{
public:
  virtual ~IModuleDevice() = default;

  virtual bool start() = 0;

  virtual void stop() noexcept = 0;

  virtual bool read(
    std::span<float> values) = 0;
};

}

#endif
