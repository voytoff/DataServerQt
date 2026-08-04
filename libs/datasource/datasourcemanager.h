#pragma once

#include "iactivedatasource.h"
#include <memory>
#include <vector>

namespace qds
{

class DataSourceManager
{
public:
  bool add(std::unique_ptr<IActiveDataSource> source);

  [[nodiscard]] bool start();
  void stop() noexcept;

  [[nodiscard]] bool step();

  [[nodiscard]]
  bool isRunning() const noexcept;

  [[nodiscard]]
  std::size_t size() const noexcept;

private:
  bool m_running = false;
  std::vector<std::unique_ptr<IActiveDataSource>> m_sources;
};

}