#pragma once

#include "formulaastrepository.h"
#include "signalmemorylayout.h"
#include "systemconfiguration.h"

namespace qds
{

class FormulaBuilder
{
public:

  bool build(
    SystemConfiguration& configuration,
    const SignalMemoryLayout& layout,
    FormulaAstRepository& formulas);

};

}