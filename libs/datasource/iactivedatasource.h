#pragma once

namespace qds
{

/** общий интерфейс источника данных */
class IActiveDataSource
{
public:
  virtual ~IActiveDataSource() noexcept = default;

  [[nodiscard]]
  virtual bool start() = 0;

  virtual void stop() noexcept = 0;

  [[nodiscard]] // immidiate
  virtual bool step() = 0;

  [[nodiscard]]
  virtual bool isRunning() const noexcept = 0;
};

}