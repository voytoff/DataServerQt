#pragma once

#include "imodulegenerator.h"

namespace qds
{

class CounterGenerator : public IModuleGenerator
{
public:
  void generate(ModuleId, std::span<float> values) override;

private:
  float m_counter = 0.0f;
};

}