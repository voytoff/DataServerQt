#pragma once

#include "signalmemory.h"

namespace qds
{

/** общий интерфейс источника данных */
class IDataSource
{
public:
  virtual ~IDataSource() = default;

  [[nodiscard]]
  virtual bool start() = 0;

  virtual void stop() noexcept = 0;

  [[nodiscard]] // immidiate
  virtual bool step() = 0;
  [[nodiscard]]
  virtual bool read(RawMemory& memory) = 0;

  [[nodiscard]]
  virtual bool isRunning() const noexcept = 0;
};

}