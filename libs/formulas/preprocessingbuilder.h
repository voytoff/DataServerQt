#pragma once

#include "formulaastrepository.h"
#include "systemconfiguration.h"

namespace qds
{

class PreprocessingBuilder
{
public:
  bool build(
    SystemConfiguration& configuration,
    FormulaAstRepository& formulas);
};

}