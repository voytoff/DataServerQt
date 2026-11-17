#include "applicationsettingsloader.h"
#include "databaseconnection.h"
#include "mainwindow.h"
#include "colorschemesettings.h"
#include "applicationsettingsdialog.h"

#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QStyleFactory>
#include <QTranslator>

static bool showError(
  const QString& message)
{
  qCritical().noquote() << message;

  QMessageBox::critical(
    nullptr,
    QObject::tr("ConfigEditor — ошибка запуска"),
    message);

  return false;
}

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  app.setApplicationName("ConfigEditor");

  QTranslator qtTranslator;

  const QString lang =
    QLocale::system().name();

  if (qtTranslator.load(
        "qt_" + lang,
        QLibraryInfo::path(
          QLibraryInfo::TranslationsPath)))
  {
    app.installTranslator(&qtTranslator);
  }

  QTranslator myTranslator;

  if (myTranslator.load(
        "configeditor_" + lang,
        ":/translations"))
  {
    app.installTranslator(&myTranslator);
  }

  app.setStyle(
    QStyleFactory::create("windows11"));




  qds::ApplicationSettings settings;
  QSqlDatabase database;

  const QString configFile =
    qds::ApplicationSettingsLoader::configFileName(app);

  if (configFile.isEmpty())
  {
    showError(QObject::tr("Не удалось определить путь к настройкам"));
    return 1;
  }

  if (!QDir().mkpath(QFileInfo(configFile).absolutePath()))
  {
    showError(QObject::tr("Не удалось создать каталог настроек"));
    return 1;
  }

  while (true)
  {
    QString error;

    const bool loaded =
      qds::ApplicationSettingsLoader::load(
        configFile,
        settings,
        error,
        qds::SettingsSection::Database);

    if (loaded &&
        qds::DatabaseConnection::open(
          settings.database,
          database,
          error))
    {
      break;
    }

    qWarning().noquote() << error;

    // При ошибке загрузки получаем значения по умолчанию.
    // При ошибке подключения сохраняем загруженные настройки.
    if (!loaded)
    {
      settings = qds::ApplicationSettings{};
      settings.configFile = configFile;
    }

    qds::ApplicationSettingsDialog dialog(
      settings,
      qds::SettingsSection::Database);

    if (dialog.exec() != QDialog::Accepted)
      return 0;
  }





  qds::ColorSchemeSettings::restore(
    settings.configFile);

  qds::MainWindow window(
    database,
    settings.configFile,
    settings.configurationId);

  window.show();

  return app.exec();
}