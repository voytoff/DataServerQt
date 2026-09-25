#pragma once

#include "datastreamsourcefactory.h"
#include "systemconfiguration.h"

#include <memory>
#include <vector>

namespace qds
{

class DataStreamSourceManager
{
public:

  [[nodiscard]]
  bool initialize(
    const SystemConfiguration& configuration,
    const DataStreamSourceFactory& factory);

  [[nodiscard]]
  bool start() noexcept;

  void stop() noexcept;

  [[nodiscard]]
  bool isRunning() const noexcept;

  [[nodiscard]]
  std::size_t size() const noexcept;

private:

  std::vector<std::unique_ptr<IDataStreamSource>> m_sources;

  bool m_running = false;
};

}