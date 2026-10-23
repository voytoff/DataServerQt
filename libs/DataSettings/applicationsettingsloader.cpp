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
  QString &error,
  bool requireServerSettings)
{
  error.clear();

  QCommandLineParser parser;
  parser.setApplicationDescription(
    app.applicationName());

  parser.addHelpOption();

  QCommandLineOption configOption(
    "config",
    "DataMonitor INI file",
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

  // ConfigurationId читаем всегда, если он присутствует.

  s.beginGroup("Server");

  bool idOk = false;

  const auto configurationId =
    s.value("ConfigurationId").toUInt(&idOk);

  s.endGroup();

  ApplicationSettings result;

  result.configFile = configFile;
  result.database = std::move(database);

  if (idOk && configurationId != 0)
  {
    result.configurationId =
      ConfigurationId{configurationId};
  }
  else
  {
    error = "Некорректный ConfigurationId";
    return false;
  }

  if (requireServerSettings)
  {
    s.beginGroup("Archive");

    const QString archive =
      s.value("Directory").toString();

    s.endGroup();

    if (archive.isEmpty())
    {
      error = "Некорректный каталог архивирования";
      return false;
    }

    s.beginGroup("Log");

    const QString log =
      s.value("Directory").toString();

    s.endGroup();

    if (log.isEmpty())
    {
      error = "Некорректный каталог логирования";
      return false;
    }

#ifdef Q_OS_WIN
    result.archiveDirectory =
      std::filesystem::path(archive.toStdWString());

    result.logDirectory =
      std::filesystem::path(log.toStdWString());
#else
    result.archiveDirectory =
      std::filesystem::u8path(
        archive.toUtf8().constData());

    result.logDirectory =
      std::filesystem::u8path(
        log.toUtf8().constData());
#endif
  }

  if (s.status() != QSettings::NoError)
  {
    error =
      "Ошибка чтения файла конфигурации:\n" +
      configFile;

    return false;
  }

  settings = std::move(result);

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
