#include "archivesignalpoint.h"

#include <QGuiApplication>

int main(int argc, char *argv[])
{
  QGuiApplication app(argc, argv);

  qRegisterMetaType<qds::ArchiveSignalSegments>(
    "qds::ArchiveSignalSegments");

  return QGuiApplication::exec();
}

Q_DECLARE_METATYPE(qds::ArchiveSignalSegments);

