#ifndef MEMORYSTORAGE_H
#define MEMORYSTORAGE_H

#include "tag.h"

class MemoryStorage
{
public:
  MemoryStorage();

  void resize(TagId count);
  double value(TagId id) const;
  void setValue(TagId id, double value);

  void write(TagId first,
             const double *values,
             size_t count);

private:

  std::vector<double> m_values;
};

#endif // MEMORYSTORAGE_H
