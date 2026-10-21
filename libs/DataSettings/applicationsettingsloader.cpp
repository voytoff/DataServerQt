#include "applicationsettingsloader.h"

#include <QSettings>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>

namespace qds
{


bool ApplicationSettingsLoader::load(
  const QCoreApplication &app,
  ApplicationSettings &settings,
  QString &error)
{
  error.clear();

  QCommandLineParser parser;
  parser.setApplicationDescription(
    app.applicationName());

  parser.addHelpOption();

  QCommandLineOption configOption(
    "config",
    "Configuration INI file",
    "file",
    "dataserver.ini");

  parser.addOption(configOption);
  parser.process(app);

  const QString configFile =
    parser.value(configOption);

  if (!QFileInfo::exists(configFile))
  {
    error = "Не найден файл конфигурации:\n" + configFile;
    return false;
  }

  QSettings s(
    configFile,
    QSettings::IniFormat);

  DatabaseSettings database;

  s.beginGroup("Database");

  database.plugin =
    s.value("Plugin", "QMARIADB").toString();

  database.host =
    s.value("Host", "127.0.0.1").toString();

  database.port =
    s.value("Port", 3306).toInt();

  database.name =
    s.value("Name").toString();

  database.user =
    s.value("User").toString();

  database.password =
    s.value("Password").toString();

  s.endGroup();

  if (s.status() != QSettings::NoError ||
      database.name.isEmpty() ||
      database.user.isEmpty() ||
      database.port < 1 ||
      database.port > 65535)
  {
    error =
      "Неверные настройки доступа к базе данных в файле конфигурации:\n" +
      configFile;

    return false;
  }

  s.beginGroup("Server");

  bool idOk = false;

  const auto configurationId =
    s.value("ConfigurationId")
      .toUInt(&idOk);

  s.endGroup();

  if (!idOk || configurationId == 0)
  {
    error = "Некорректный ConfigurationId";
    return false;
  }

  s.beginGroup("Archive");

  const auto archive =
    s.value("Directory")
      .toString();

  s.endGroup();

  if (archive.isEmpty())
  {
    error = "Некорректный каталог архивирования";
    return false;
  }

  s.beginGroup("Log");

  const auto log =
    s.value("Directory")
      .toString();

  s.endGroup();

  if (log.isEmpty())
  {
    error = "Некорректный каталог логирования";
    return false;
  }

  if (s.status() != QSettings::NoError)
  {
    error =
      "Ошибка чтения файла конфигурации:\n" +
      configFile;

    return false;
  }

  settings = ApplicationSettings{
    .configFile = configFile,
    .database = std::move(database),
    .configurationId = ConfigurationId{configurationId},
    .archiveDirectory = std::filesystem::path(archive.toStdString()),
    .logDirectory = std::filesystem::path(log.toStdString())
  };

  return true;
}

bool ApplicationSettingsLoader::saveConfigurationId(
  const QString& fileName,
  ConfigurationId id,
  const QString& key)
{
  if (id.value == 0)
    return false;

  QSettings settings(
    fileName,
    QSettings::IniFormat);

  settings.setValue(key, id.value);
  settings.sync();

  return settings.status() == QSettings::NoError;
}

}
