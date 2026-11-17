#pragma once

#include <QString>
#include <QtGlobal>

#include <filesystem>

namespace qds
{

inline QString pathToQString(
  const std::filesystem::path& path)
{
#ifdef Q_OS_WIN
  return QString::fromStdWString(path.wstring());
#else
  return QString::fromUtf8(path.native());
#endif
}

}