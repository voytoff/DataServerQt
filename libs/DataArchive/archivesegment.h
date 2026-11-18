#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>

namespace qds
{

std::filesystem::path segmentFileName(
  const std::filesystem::path& name,
  uint64_t index);

std::optional<uint64_t> parseSegmentFileName(
  const std::filesystem::path& name,
  const std::filesystem::path& segment);

}