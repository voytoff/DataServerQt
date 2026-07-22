#ifndef IDATASOURCE_H
#define IDATASOURCE_H

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

  [[nodiscard]]
  virtual bool step() = 0;

  [[nodiscard]]
  virtual bool isRunning() const noexcept = 0;
};

}

#endif // IDATASOURCE_H
