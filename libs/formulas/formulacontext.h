#pragma once

#include <span>

namespace qds
{

struct FormulaContext
{
  double* output = nullptr;

  std::span<double*> inputs;
};

}