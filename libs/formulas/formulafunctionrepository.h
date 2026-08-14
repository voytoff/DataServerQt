#pragma once

#include "iformulafunction.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace qds
{

class FormulaFunctionRepository
{
public:

  bool add(
    std::string_view name,
    std::unique_ptr<IFormulaFunction> function);

  [[nodiscard]]
  const IFormulaFunction* find(
    std::string name) const;

  void clear();

  [[nodiscard]]
  std::size_t size() const noexcept;

private:

  std::unordered_map<
    std::string,
    std::unique_ptr<IFormulaFunction>> m_functions;
};

}