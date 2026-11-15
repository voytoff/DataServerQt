
#pragma once

#include "archivetablewriter.h"
#include "dataexport_global.h"

#include <cstdint>
#include <filesystem>
#include <memory>

namespace qds
{

class DATAEXPORT_EXPORT XlsxTableWriter final
  : public ArchiveTableWriter
{
public:
  explicit XlsxTableWriter(
    std::filesystem::path outputPath);

  ~XlsxTableWriter() override;

  XlsxTableWriter(const XlsxTableWriter&) = delete;
  XlsxTableWriter& operator=(const XlsxTableWriter&) = delete;

  bool begin(
    const std::vector<QString>& headers) override;

  bool writeRow(
    double seconds,
    int64_t wallMilliseconds,
    std::span<const std::optional<double>> values) override;

  bool finish() override;

  void abort() noexcept override;

  [[nodiscard]]
  QString errorString() const override;

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

}
