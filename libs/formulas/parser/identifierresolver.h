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
    FormulaNode& root);

private:

  bool resolveNode(
    FormulaNode& node);

private:

  const SystemConfiguration& m_cfg;
  const SignalMemoryLayout& m_layout;
};

}