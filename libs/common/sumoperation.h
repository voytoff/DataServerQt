#ifndef SUMOPERATION_H
#define SUMOPERATION_H

#include "operation.cpp"

namespace qds
{
class SumOperation : public Operation
{
public:

  SumOperation(std::vector<TagId> inputs,
               TagId output)
  {
    m_inputs = std::move(inputs);
    m_outputs = { output };
  }

  void execute(double* memory) override
  {
    double sum = 0.0;

    for (auto id : m_inputs)
      sum += memory[id];

    memory[m_outputs[0]] = sum;
  }
};

}

#endif // SUMOPERATION_H
