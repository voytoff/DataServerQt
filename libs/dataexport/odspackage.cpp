#include "odspackage.h"
#include "pathutils.h"

#include <QFile>

#include <zip.h>

namespace qds
{

OdsPackage::OdsPackage(std::filesystem::path outputPath)
  : m_outputPath(std::move(outputPath))
{
}

OdsPackage::~OdsPackage()
{
  abort();
}

bool OdsPackage::begin()
{
  m_error.clear();

  if (m_archive)
  {
    m_error = QStringLiteral("Архив уже открыт");
    return false;
  }

  int errorCode = 0;

  const QByteArray path =
    QFile::encodeName(pathToQString(m_outputPath));

  m_archive = zip_open(
    path.constData(),
    ZIP_CREATE | ZIP_TRUNCATE,
    &errorCode);

  if (!m_archive)
  {
    zip_error_t zipError;
    zip_error_init_with_code(&zipError, errorCode);

    m_error = QString::fromUtf8(
      zip_error_strerror(&zipError));

    zip_error_fini(&zipError);

    return false;
  }

  return true;
}

bool OdsPackage::addData(
  const QString& name,
  QByteArray data,
  bool compress)
{
  if (!m_archive)
  {
    m_error = QStringLiteral("Архив не открыт");
    return false;
  }

  m_buffers.push_back(std::move(data));

  const QByteArray& buffer = m_buffers.back();

  zip_source_t* source = zip_source_buffer(
    m_archive,
    buffer.constData(),
    static_cast<zip_uint64_t>(buffer.size()),
    0);

  const QByteArray fileName = name.toUtf8();

  if (!source)
  {
    m_error = QString::fromUtf8(
      zip_strerror(m_archive));
    return false;
  }

  const zip_int64_t index = zip_file_add(
    m_archive,
    fileName.constData(),
    source,
    ZIP_FL_ENC_UTF_8);

  if (index < 0)
  {
    m_error = QString::fromUtf8(
      zip_strerror(m_archive));

    zip_source_free(source);
    return false;
  }

  if (zip_set_file_compression(
        m_archive,
        static_cast<zip_uint64_t>(index),
        compress ? ZIP_CM_DEFLATE : ZIP_CM_STORE,
        0) < 0)
  {
    m_error = QString::fromUtf8(
      zip_strerror(m_archive));

    return false;
  }

  return true;
}

bool OdsPackage::addFile(
  const QString& name,
  const QString& filePath,
  bool compress)
{
  if (!m_archive)
  {
    m_error = QStringLiteral("Архив не открыт");
    return false;
  }

  const QByteArray fileName = name.toUtf8();
  const QByteArray sourcePath = QFile::encodeName(filePath);

  zip_source_t* source = zip_source_file(
    m_archive,
    sourcePath.constData(),
    0,
    0);

  if (!source)
  {
    m_error = QString::fromUtf8(
      zip_strerror(m_archive));
    return false;
  }

  const zip_int64_t index = zip_file_add(
    m_archive,
    fileName.constData(),
    source,
    ZIP_FL_ENC_UTF_8);

  if (index < 0)
  {
    m_error = QString::fromUtf8(
      zip_strerror(m_archive));

    zip_source_free(source);
    return false;
  }

  if (zip_set_file_compression(
        m_archive,
        static_cast<zip_uint64_t>(index),
        compress ? ZIP_CM_DEFLATE : ZIP_CM_STORE,
        0) < 0)
  {
    m_error = QString::fromUtf8(
      zip_strerror(m_archive));

    return false;
  }

  return true;
}

bool OdsPackage::finish()
{
  if (!m_archive)
  {
    m_error = QStringLiteral("Архив не открыт");
    return false;
  }

  zip_t* archive = m_archive;
  m_archive = nullptr;

  if (zip_close(archive) < 0)
  {
    m_error = QString::fromUtf8(
      zip_strerror(archive));

    zip_discard(archive);
    m_buffers.clear();

    return false;
  }

  m_buffers.clear();
  return true;
}

void OdsPackage::abort() noexcept
{
  if (m_archive)
  {
    zip_discard(m_archive);
    m_archive = nullptr;
  }

  m_buffers.clear();
}

QString OdsPackage::errorString() const
{
  return m_error;
}

}