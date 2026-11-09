#include "dataviewermainwindow.h"
#include "archivesignalpoint.h"

#include <QApplication>

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  qRegisterMetaType<qds::ArchiveSignalSegments>(
    "qds::ArchiveSignalSegments");

  qds::DataViewerMainWindow window;

  window.resize(1200, 800);
  window.show();

  return app.exec();
}