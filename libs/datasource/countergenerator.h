#ifndef COUNTERGENERATOR_H
#define COUNTERGENERATOR_H

#include "imodulegenerator.h"

namespace qds
{

class CounterGenerator : public IModuleGenerator
{
public:
  void generate(ModuleId module, std::span<float> values) override;

private:
  float m_counter = 0.0f;
};

}

#endif // COUNTERGENERATOR_H
