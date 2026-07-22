#ifndef DATASOURCEMANAGER_H
#define DATASOURCEMANAGER_H

#include "idatasource.h"
#include <memory>
#include <vector>

namespace qds
{

class DataSourceManager
{
public:
  bool add(std::unique_ptr<IDataSource> source);

  [[nodiscard]] bool start();
  void stop() noexcept;

  [[nodiscard]] bool step();

  [[nodiscard]]
  bool isRunning() const noexcept;

  [[nodiscard]]
  std::size_t size() const noexcept;

private:
  bool m_running = false;
  std::vector<std::unique_ptr<IDataSource>> m_sources;
};

}

#endif // DATASOURCEMANAGER_H
