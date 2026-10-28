#pragma once

#include "themedsvgiconengine.h"

#include <QIcon>

namespace qds
{

inline QIcon themedSvgIcon(
  const QString& path)
{
  return QIcon(
    new ThemedSvgIconEngine(path));
}

}