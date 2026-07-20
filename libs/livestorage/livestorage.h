#ifndef LIVESTORAGE_H
#define LIVESTORAGE_H

#include "datatypes.h"
#include <cstdint>
#include <span>
#include <vector>
//#include "systemconfiguration.h"

namespace qds
{

class SystemConfiguration;
class LiveStorage
{
public:
  explicit LiveStorage(const SystemConfiguration& cfg);

  [[nodiscard]]
  bool updateModule(ModuleId module, std::span<const float> values, uint64_t timestamp);

  [[nodiscard]] const Sample& sample(TagId id) const;
  bool read(TagId tag, Sample& sample) const;

  [[nodiscard]] uint64_t moduleTimestamp(ModuleId id) const noexcept;

  [[nodiscard]] std::size_t size() const noexcept;

private:
  const SystemConfiguration& m_cfg;

  std::vector<Sample>   m_samples;
  std::vector<uint64_t> m_moduleTimestamps;
};
}

#endif // LIVESTORAGE_H
