#pragma once

#include "datastreamsourcestatistics.h"
namespace qds
{

class IDataStreamSource
{
public:

  virtual ~IDataStreamSource() noexcept = default;

  [[nodiscard]]
  virtual bool start() noexcept = 0;

  virtual void stop() noexcept = 0;

  [[nodiscard]]
  virtual bool isRunning() const noexcept = 0;

  virtual DataStreamSourceStatistics statistics() const noexcept = 0;
};

}