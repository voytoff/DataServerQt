#pragma once

#include "archivetablewriter.h"

#include <filesystem>
#include <memory>

namespace qds
{

class OdsTableWriter final : public ArchiveTableWriter
{
public:
  explicit OdsTableWriter(
    std::filesystem::path fileName);

  ~OdsTableWriter() override;

  bool begin(
    const std::vector<QString>& headers) override;

  bool writeRow(
    double seconds,
    int64_t wallMilliseconds,
    std::span<const std::optional<double>> values) override;

  bool finish() override;

  void abort() noexcept override;

  QString errorString() const override;

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

}