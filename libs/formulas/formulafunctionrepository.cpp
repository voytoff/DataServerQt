#include <cassert>
#include "formulafunctionrepository.h"

namespace qds
{

bool FormulaFunctionRepository::add(
  std::string_view name,
  std::unique_ptr<IFormulaFunction> function)
{
  if (name.empty())
    return false;

  if (!function)
    return false;

  const auto functionName = std::string(name);

  if (find(functionName) != nullptr)
    return false;

  auto [it, inserted] =
    m_functions.emplace(
      functionName,
      std::move(function));

  return inserted;
}

const IFormulaFunction*
FormulaFunctionRepository::find(std::string name) const
{
  auto it = m_functions.find(name);

  if (it == m_functions.end())
    return nullptr;

  return it->second.get();
}

void FormulaFunctionRepository::clear()
{
  m_functions.clear();
}

std::size_t FormulaFunctionRepository::size() const noexcept
{
  return m_functions.size();
}

}
