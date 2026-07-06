#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <algorithm>
#include "datatypes.h"

namespace qds
{

class Memory
{
public:


  double* data() { return m_data.data(); }
  size_t size() const { return m_data.size(); }


  void resize(TagId size)
  {
    m_data.assign(size, 0.0);
  }

  double get(TagId id) const
  {
    return m_data[id];
  }

  void set(TagId id, double value)
  {
    m_data[id] = value;
  }

  // 🔥 самое важное
  void write(TagId first,
             const double* values,
             size_t count)
  {
    std::copy(values,
              values + count,
              m_data.begin() + first);
  }

  double* raw()
  {
    return m_data.data();
  }

  const double* raw() const
  {
    return m_data.data();
  }

private:
  std::vector<double> m_data;
};

}

#endif // MEMORY_H
