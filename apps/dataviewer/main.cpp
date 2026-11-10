#include "dataviewermainwindow.h"
#include "archivesignalpoint.h"

#include <QApplication>
#include <QStyleFactory>
#include <QTranslator>
#include <QLibraryInfo>

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

  qds::DataViewerMainWindow window;

  window.resize(1200, 800);
  window.show();

  return app.exec();
}