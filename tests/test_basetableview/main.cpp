#include "archivesignalpoint.h"
#include "testmodule.h"

#include <QApplication>
#include <QStandardItemModel>
#include <QAbstractItemView>
#include <qmainwindow.h>
#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QLineSeries>

#include <cmath>
#include <timeaxis.h>
#include <timechartview.h>

int main(int argc, char* argv[])
{
  using namespace qds;

  QApplication app(argc, argv);

  qRegisterMetaType<qds::ArchiveSignalSegments>(
    "qds::ArchiveSignalSegments");


  TestModule tm;

  tm.createDynamicArchiveChartsView();

  return app.exec();
}

Q_DECLARE_METATYPE(qds::ArchiveSignalSegments);



