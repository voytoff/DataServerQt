#pragma once

#include "datawidgets_export.h"

#include <QColor>

#include <cstddef>
#include <cstdint>

namespace qds
{

class DATAWIDGETS_EXPORT ChartPalette
{
public:
  static std::size_t size() noexcept;

  static QColor color(std::size_t index);

  static QColor signalColor(uint32_t signalId);
};

}