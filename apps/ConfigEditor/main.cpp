#include "mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStyleFactory>
#include <QLibraryInfo>
#include <QTranslator>

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  QTranslator qtTranslator;
  QString lang = QLocale::system().name();
  // Загружаем системный перевод (ищет файлы вида qt_ru.qm)
  if (qtTranslator.load("qt_" + lang, QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
    app.installTranslator(&qtTranslator);

  // Здесь загружается перевод для собственного приложения
  QTranslator myTranslator;
  if (myTranslator.load("acdanalizer_" + lang, ":/translations"))
    app.installTranslator(&myTranslator);

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