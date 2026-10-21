#include "databaseconnection.h"
#include <QSqlError>

namespace qds
{

bool DatabaseConnection::open(
  const DatabaseSettings& settings,
  QSqlDatabase& database,
  QString& error)
{
  error.clear();

  QSqlDatabase db =
    QSqlDatabase::addDatabase(settings.plugin);

  db.setHostName(settings.host);
  db.setPort(settings.port);
  db.setDatabaseName(settings.name);
  db.setUserName(settings.user);
  db.setPassword(settings.password);

  if (!db.open())
  {
    error =
      "Не удалось подключиться к базе данных:\n" +
      db.lastError().text();

    return false;
  }

  database = db;
  error.clear();

  return true;
}

}
