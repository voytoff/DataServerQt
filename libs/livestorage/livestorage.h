#ifndef LIVESTORAGE_H
#define LIVESTORAGE_H

#include <cstdint>
#include <span>
#include <vector>
#include "systemconfiguration.h"

namespace qds
{

class LiveStorage
{
public:
  explicit LiveStorage(const SystemConfiguration& cfg);

  void updateModule(ModuleId module,
                    std::span<const float> values,
                    uint64_t timestamp);

  [[nodiscard]] const Sample& sample(TagId id) const;

  [[nodiscard]] uint64_t moduleTimestamp(ModuleId id) const;

  [[nodiscard]] size_t size() const;

private:
  const SystemConfiguration& m_cfg;

  std::vector<Sample>   m_samples;
  std::vector<uint64_t> m_moduleTimestamps;
};
}

#endif // LIVESTORAGE_H
