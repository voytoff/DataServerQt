#ifndef DATAENGINE_H
#define DATAENGINE_H

#include <vector>
#include <memory>

#include "datatypes.h"
#include "memory.h"
#include "operation.h"

namespace qds
{

class DataEngine
{
public:

  void resize(TagId size)
  {
    m_memory.resize(size);
  }

  Memory& memory()
  {
    return m_memory;
  }

  const Memory& memory() const
  {
    return m_memory;
  }

  void addOperation(std::unique_ptr<Operation> op)
  {
    m_operations.emplace_back(std::move(op));
  }

  // 📌 только запись данных (LTR ingestion)
  void writeBlock(TagId first,
                  const double* values,
                  size_t count)
  {
    m_memory.write(first, values, count);
  }

  // 📌 вычисления (вызывается сервером по таймеру)
  void process()
  {
    double* mem = m_memory.data();

    for (auto& op : m_operations)
    {
      op->execute(mem);
    }
  }

private:
  Memory m_memory;
  std::vector<std::unique_ptr<Operation>> m_operations;
};

}

#endif // DATAENGINE_H
