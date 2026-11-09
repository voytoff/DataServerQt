#include "chartpalette.h"

#include <array>

namespace qds
{

namespace
{

const std::array<QColor, 8> Colors = {
  QColor(57, 120, 242),
  QColor(237, 98, 88),
  QColor(39, 173, 114),
  QColor(182, 122, 224),
  QColor(227, 166, 61),
  QColor(39, 181, 194),
  QColor(220, 115, 170),
  QColor(148, 166, 75)
};

}

std::size_t ChartPalette::size() noexcept
{
  return Colors.size();
}

QColor ChartPalette::color(std::size_t index)
{
  return Colors[index % Colors.size()];
}

QColor ChartPalette::signalColor(uint32_t signalId)
{
  return color(signalId);
}

}