#include <QCoreApplication>
#include <QTcpSocket>
#include <QDataStream>
#include <QTimer>
#include <QElapsedTimer>

#include "client.h"

using namespace qds;

int main(int argc, char *argv[]) {

  QCoreApplication a(argc, argv);

  Client c;
  qInfo() << "Started";

  return a.exec();
}