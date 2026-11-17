#include "applicationinitializer.h"

#include "applicationsettingsdialog.h"
#include "applicationsettingsloader.h"
#include "databaseconnection.h"

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

namespace qds
{

bool initializeApplication(
  QApplication& app,
  ApplicationSettings& settings,
  QSqlDatabase& database,
  SettingsSections sections)
{
  const QString configFile =
    ApplicationSettingsLoader::configFileName(app);

  if (configFile.isEmpty())
  {
    QMessageBox::critical(
      nullptr,
      QObject::tr("Ошибка запуска"),
      QObject::tr("Не удалось определить путь к настройкам"));

    return false;
  }

  if (!QDir().mkpath(
        QFileInfo(configFile).absolutePath()))
  {
    QMessageBox::critical(
      nullptr,
      QObject::tr("Ошибка запуска"),
      QObject::tr("Не удалось создать каталог настроек"));

    return false;
  }

  while (true)
  {
    QString error;

    const bool loaded =
      ApplicationSettingsLoader::load(
        configFile,
        settings,
        error,
        sections);

    if (loaded &&
        DatabaseConnection::open(
          settings.database,
          database,
          error))
    {
      return true;
    }

    qWarning().noquote() << error;

    if (!loaded)
    {
      settings = ApplicationSettings{};
      settings.configFile = configFile;
    }

    ApplicationSettingsDialog dialog(
      settings,
      sections,
      nullptr,
      !QFileInfo::exists(configFile));

    if (dialog.exec() != QDialog::Accepted)
      return false;
  }
}

}