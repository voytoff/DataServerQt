#pragma once

#include <QString>

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace qds
{

class ArchiveTableWriter
{
public:
  virtual ~ArchiveTableWriter() = default;

  virtual bool begin(
    const std::vector<QString>& headers) = 0;

  virtual bool writeRow(
    double seconds,
    int64_t wallMilliseconds,
    std::span<const std::optional<double>> values) = 0;

  virtual bool finish() = 0;

  virtual void abort() noexcept = 0;

  [[nodiscard]]
  virtual QString errorString() const = 0;
};

}