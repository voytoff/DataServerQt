#pragma once

#include <strongidhash.h>
#include <memory>
#include <unordered_map>
#include "iformula.h"
#include "signaldefinition.h"

namespace qds
{

class FormulaRepository
{
public:

  bool add(
    FormulaId id,
    std::unique_ptr<IFormula> formula);

  const IFormula* find(
    FormulaId id) const;

  void clear();

private:

  std::unordered_map<
    FormulaId,
    std::unique_ptr<IFormula>> m_formulas;
};

}