#pragma once

#include "formulanode.h"
#include "signalmemorylayout.h"
#include "systemconfiguration.h"

namespace qds
{

class IdentifierResolver
{
public:
  IdentifierResolver(
    const SystemConfiguration& cfg,
    const SignalMemoryLayout& layout);

  [[nodiscard]]
  const std::string& lastError() const noexcept;

  bool resolve(
    FormulaNode& root,
    std::vector<SignalId>& dependencies);

private:
  bool resolveNode(
    FormulaNode& node,
      std::vector<SignalId>& dependencies);

  bool fail(std::string message);

private:
  const SystemConfiguration& m_cfg;
  const SignalMemoryLayout& m_layout;

  std::string m_lastError;
};

}