#include "colorschemesettings.h"

#include <QApplication>
#include <QSettings>
#include <QStyleHints>

namespace qds
{

void ColorSchemeSettings::restore(
  const QString& configFile)
{
  QSettings settings(
    configFile,
    QSettings::IniFormat);

  const QString value =
    settings.value("Appearance/ColorScheme")
      .toString();

  auto* hints = QApplication::styleHints();

  if (value == "Dark")
  {
    hints->setColorScheme(
      Qt::ColorScheme::Dark);
  }
  else if (value == "Light")
  {
    hints->setColorScheme(
      Qt::ColorScheme::Light);
  }
  else
  {
    hints->setColorScheme(
      Qt::ColorScheme::Unknown);
  }
}

void ColorSchemeSettings::save(
  const QString& configFile)
{
  const auto scheme =
    QApplication::styleHints()->colorScheme();

  QString value;

  switch (scheme)
  {
  case Qt::ColorScheme::Dark:
    value = "Dark";
    break;

  case Qt::ColorScheme::Light:
    value = "Light";
    break;

  default:
    value = "System";
    break;
  }

  QSettings settings(
    configFile,
    QSettings::IniFormat);

  settings.setValue(
    "Appearance/ColorScheme",
    value);
}

}