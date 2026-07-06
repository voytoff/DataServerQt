#ifndef OPERATION_H
#define OPERATION_H

#pragma once

#include <vector>
#include "DataTypes.h"

namespace qds
{

class Operation
{
public:
  virtual ~Operation() = default;

  // выполняется всегда
  virtual void execute(double* memory) = 0;

  // входы (для будущей оптимизации/анализа)
  const std::vector<TagId>& inputs() const { return m_inputs; }

  // выходы
  const std::vector<TagId>& outputs() const { return m_outputs; }

protected:
  std::vector<TagId> m_inputs;
  std::vector<TagId> m_outputs;
};

}

#endif // OPERATION_H
