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

  bool resolve(
    FormulaNode& root,
    std::vector<SignalId>& dependencies);

private:

bool resolveNode(
  FormulaNode& node,
    std::vector<SignalId>& dependencies);

private:

  const SystemConfiguration& m_cfg;
  const SignalMemoryLayout& m_layout;
};

}