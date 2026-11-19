#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace qds
{

struct LCardDiscoveredModule
{
  uint32_t slot = 0;
  uint32_t typeId = 0;
  std::string serial;
};

struct LCardDiscoveredCrate
{
  std::string serial;
  uint32_t typeId = 0;
  uint32_t interfaceId = 0;
  std::vector<LCardDiscoveredModule> modules;
  std::string error;
};

struct LCardDiscoveryResult
{
  std::string serverVersion;
  std::vector<LCardDiscoveredCrate> crates;
  std::string error;

  [[nodiscard]] bool success() const noexcept
  {
    return error.empty();
  }
};

class LCardDiscovery final
{
public:
  [[nodiscard]]
  static LCardDiscoveryResult scan(
    uint32_t address,
    uint16_t port);

  [[nodiscard]]
  static std::string moduleName(
    uint32_t typeId);
};

} // namespace qds