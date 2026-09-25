#pragma once

namespace qds
{

class IDataStreamSource
{
public:

  virtual ~IDataStreamSource() noexcept = default;

  [[nodiscard]]
  virtual bool start() noexcept = 0;

  virtual void stop() noexcept = 0;
};

}