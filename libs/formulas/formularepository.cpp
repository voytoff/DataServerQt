#include "formularepository.h"

namespace qds
{

bool FormulaRepository::add(
  FormulaId id,
  std::unique_ptr<IFormula> formula)
{
  auto [it, inserted] =
    m_formulas.emplace(id, std::move(formula));

  return inserted;
}

const IFormula*
FormulaRepository::find(
  FormulaId id) const
{
  auto it = m_formulas.find(id);

  if (it == m_formulas.end())
    return nullptr;

  return it->second.get();
}

void FormulaRepository::clear()
{
  m_formulas.clear();
}

}