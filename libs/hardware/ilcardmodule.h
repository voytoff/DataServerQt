#pragma once

#include <span>

namespace qds
{

class ILCardModule
{
public:
  virtual ~ILCardModule() noexcept = default;

  virtual bool start() noexcept = 0;
  virtual void stop() noexcept = 0;

  // Возвращает последние полученные значения.
  // Непосредственный опрос оборудования здесь не выполняется.
  virtual bool read(std::span<double> values) noexcept = 0;
};

}