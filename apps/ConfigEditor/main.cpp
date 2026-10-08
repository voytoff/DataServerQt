#include "mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStyleFactory>

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  // "windows11", "windowsvista", "Windows", "Fusion"
  app.setStyle(QStyleFactory::create("windows11"));

  QSqlDatabase database =
    QSqlDatabase::addDatabase(
      "QMYSQL");

  database.setHostName("127.0.0.1");
  database.setPort(3306);
  database.setDatabaseName("dataserver");

  // Пока тестовые параметры.
  database.setUserName("root");
  database.setPassword("1234");

  if (!database.open())
  {
    QMessageBox::critical(
      nullptr,
      "Database error",
      database.lastError().text());

    return 1;
  }

  qds::MainWindow window(database);
  window.show();

  return app.exec();
}