#include "calibrationdialog.h"
#include "applicationsettingsdialog.h"
#include "applicationsettingsloader.h"
#include "settingssection.h"

#include <QApplication>
#include <QStandardItemModel>
#include <QAbstractItemView>

void calibrationDialog()
{
  using namespace qds;

  Calibration calibration;

  calibration.kind = CalibrationKind::Polynomial;
  calibration.description = "Проверка редактора";

  if (calibration.kind == CalibrationKind::Table)
  {
    calibration.points = {
                          {0, 0.0, 0.0},
                          };
  }
  else
  {
    calibration.coefficients = {
      0
    };
  }
  CalibrationDialog dialog(calibration, nullptr);

  if (dialog.exec() != QDialog::Accepted)
    return;

  const Calibration result =
    dialog.calibration();

  if (calibration.kind == CalibrationKind::Table)
  {
    qDebug() << "Calibration:"
             << result.description
             << "Points:" << result.points.size();

    for (const auto& point : result.points)
      qDebug() << point.index << point.x << point.y;
  }
  else
  {
    qDebug() << "Calibration:"
             << result.description
             << "Coefficients:" << result.coefficients.size();

    for (const auto& point : result.coefficients)
      qDebug() << point;
  }
}

void applicationSettingsDialog()
{
  using namespace qds;

  const QString fileName =
    "D:/DATASERVER/ini/configeditor_test.ini";

  const SettingsSections sections =
    SettingsSection::Database |
    SettingsSection::Archive |
    SettingsSection::Log;

  ApplicationSettings settings;
  QString error;

  if (!ApplicationSettingsLoader::load(
        fileName,
        settings,
        error,
        sections))
  {
    qWarning() << error;
    return;
  }

  ApplicationSettingsDialog dialog(
    settings,
    sections);

  QObject::connect(
    &dialog,
    &ApplicationSettingsDialog::settingsApplied,
    &dialog,
    [](SettingsSections changed)
    {
      qDebug() << "Изменены разделы:" << changed;
    });

  dialog.exec();
}

int main(int argc, char* argv[])
{
  using namespace qds;

  QApplication app(argc, argv);

  applicationSettingsDialog();

  return 0;
}

