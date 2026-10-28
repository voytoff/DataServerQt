#include "colorscheme.h"

#include <QApplication>
#include <QStyleHints>

namespace qds
{

void toggleColorScheme()
{
  auto* hints =
    QApplication::styleHints();

  const auto current =
    hints->colorScheme();

  const auto next =
    current == Qt::ColorScheme::Dark
      ? Qt::ColorScheme::Light
      : Qt::ColorScheme::Dark;

  hints->setColorScheme(next);
}

}