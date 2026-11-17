#pragma once

#include <QByteArray>
#include <QString>

#include <deque>
#include <filesystem>

struct zip;

namespace qds
{

class OdsPackage final
{
public:
  explicit OdsPackage(std::filesystem::path outputPath);
  ~OdsPackage();

  OdsPackage(const OdsPackage&) = delete;
  OdsPackage& operator=(const OdsPackage&) = delete;

  bool begin();

  bool addData(
    const QString& name,
    QByteArray data,
    bool compress = true);

  bool addFile(
    const QString& name,
    const QString& filePath,
    bool compress = true);

  bool finish();

  void abort() noexcept;

  QString errorString() const;

private:
  std::filesystem::path m_outputPath;
  zip* m_archive = nullptr;
  QString m_error;
  std::deque<QByteArray> m_buffers;
};

}