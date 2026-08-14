#pragma once

#include "parser/formulanode.h"

#include <strongidhash.h>
#include <memory>
#include <unordered_map>

namespace qds
{

class FormulaAstRepository
{
public:

  bool add(
    FormulaId id,
    std::unique_ptr<FormulaNode> root);

  [[nodiscard]]
  const FormulaNode* find(
    FormulaId id) const;

  void clear();

  [[nodiscard]]
  std::size_t size() const noexcept;

private:

  std::unordered_map<
    FormulaId,
    std::unique_ptr<FormulaNode>> m_formulas;
};

}