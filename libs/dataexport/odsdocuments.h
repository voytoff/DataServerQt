#pragma once

#include <QByteArray>
#include <QString>

#include <cstdint>

namespace qds
{

namespace ods
{

inline constexpr uint32_t MaxWorksheetRows = 1'048'576;
inline constexpr uint32_t MaxWorksheetColumns = 16'384;

inline const QString nsOffice =
  QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:office:1.0");

inline const QString nsTable =
  QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:table:1.0");

inline const QString nsText =
  QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:text:1.0");

inline const QString nsStyle =
  QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:style:1.0");

inline const QString nsNumber =
  QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0");

inline const QString nsFo =
  QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");

inline const QString nsMeta =
  QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:meta:1.0");

inline const QString nsManifest =
  QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:manifest:1.0");

}

QByteArray buildMimeType();
QByteArray buildMetaXml();
QByteArray buildManifestXml();
QByteArray buildStylesXml();

}