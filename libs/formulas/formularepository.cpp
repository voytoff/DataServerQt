#include "formularepository.h"

namespace qds
{

bool FormulaRepository::add(
  FormulaId id,
  std::unique_ptr<IFormula> formula)
{
  auto exists = find(id);

  if (exists)
    return false;

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

std::size_t FormulaRepository::size() const noexcept
{
  return m_formulas.size();
}

}