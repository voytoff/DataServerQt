#include "formulaastrepository.h"

namespace qds
{

bool FormulaAstRepository::add(FormulaId id, std::unique_ptr<FormulaNode> root)
{
  if (!root)
    return false;

  if (find(id) != nullptr)
    return false;

  auto [it, inserted] =
    m_formulas.emplace(
      id,
      std::move(root));

  return inserted;
}

FormulaNode* FormulaAstRepository::find(
  FormulaId id) noexcept
{
  auto it = m_formulas.find(id);

  if (it == m_formulas.end())
    return nullptr;

  return it->second.get();
}

const FormulaNode* FormulaAstRepository::find(
  FormulaId id) const noexcept
{
  auto it = m_formulas.find(id);

  if (it == m_formulas.end())
    return nullptr;

  return it->second.get();
}

void FormulaAstRepository::clear()
{
  m_formulas.clear();
}

std::size_t FormulaAstRepository::size() const noexcept
{
  return m_formulas.size();
}

}
