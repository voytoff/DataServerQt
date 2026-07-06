#ifndef LINEAROPERATION_H
#define LINEAROPERATION_H

#include "operation.h"

namespace qds
{

class LinearOperation : public Operation
{
public:

  LinearOperation(TagId input,
                  TagId output,
                  double a,
                  double b)
    : m_input(input), m_output(output), m_a(a), m_b(b)
  {
    m_inputs = { input };
    m_outputs = { output };
  }

  void execute(double* memory) override
  {
    memory[m_output] = m_a * memory[m_input] + m_b;
  }

private:
  TagId m_input;
  TagId m_output;
  double m_a;
  double m_b;
};

}
#endif // LINEAROPERATION_H
