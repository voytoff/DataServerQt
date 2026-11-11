#include "dataviewermainwindow.h"
#include "archivesignalpoint.h"
#include "colorschemesettings.h"

#include <QApplication>
#include <QStyleFactory>
#include <QTranslator>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QDir>

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

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

  qRegisterMetaType<qds::ArchiveSignalSegments>(
    "qds::ArchiveSignalSegments");

  const QString configPath =
    QStandardPaths::writableLocation(
      QStandardPaths::AppConfigLocation);

  QDir().mkpath(configPath);

  const auto configFile =
    QDir(configPath).filePath(
      QStringLiteral("dataviewer.ini"));

  qds::ColorSchemeSettings::restore(
    configFile);

  qds::DataViewerMainWindow window(configFile);

  window.show();

  return app.exec();
}