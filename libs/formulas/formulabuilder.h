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

  [[nodiscard]]
  const std::string& lastError() const noexcept;

private:
  std::string m_lastError;

};

}