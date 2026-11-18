#include "archivesegment.h"

#include <charconv>
#include <iomanip>
#include <sstream>
#include <string>

namespace qds
{

std::filesystem::path segmentFileName(
  const std::filesystem::path& name,
  uint64_t index)
{
  std::ostringstream stream;

  stream
    << name.stem().string()
    << '_'
    << std::setw(4)
    << std::setfill('0')
    << index
    << name.extension().string();

  return stream.str();
}

std::optional<uint64_t> parseSegmentFileName(
  const std::filesystem::path& name,
  const std::filesystem::path& segment)
{
  if (name.extension() != segment.extension())
    return std::nullopt;

  const std::string prefix =
    name.stem().string() + '_';

  const std::string stem =
    segment.stem().string();

  if (!stem.starts_with(prefix))
    return std::nullopt;

  const std::string_view number(
    stem.data() + prefix.size(),
    stem.size() - prefix.size());

  if (number.size() < 4)
    return std::nullopt;

  uint64_t index = 0;

  const auto [ptr, ec] = std::from_chars(
    number.data(),
    number.data() + number.size(),
    index);

  if (ec != std::errc{} ||
      ptr != number.data() + number.size() ||
      index == 0)
  {
    return std::nullopt;
  }

  return index;
}

}