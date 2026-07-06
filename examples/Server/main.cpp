#include <QCoreApplication>
#include <QDebug>

#include "dataserver.h"
#include "dataengine.h"

#include "LinearOperation.h"

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  qds::DataEngine engine;

  // 📌 размер памяти (например 10k тегов)
  engine.resize(10000);

  // 📌 тестовая операция (для проверки pipeline)
  engine.addOperation(std::make_unique<qds::LinearOperation>(
    0, 1, 2.0, 5.0));

  qds::DataServer server(&engine);

  if (!server.start(12345))
  {
    qDebug() << "Failed to start server";
    return -1;
  }

  qDebug() << "DataServer started on port 12345";

  return app.exec();
}