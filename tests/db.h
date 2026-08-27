#pragma once

#include <QSqlDatabase>

const QString plugin = "QMARIADB";
const QString hostName = "127.0.0.1";
const int hostPort = 3306;
const QString databaseName = "dataserver";
const QString userName = "root";
const QString password = "1234";

using namespace Qt::StringLiterals;
const QString connectionName = u"sql_default_connection"_s;

inline QSqlDatabase get_db()
{
  assert(QSqlDatabase::isDriverAvailable(plugin));

  if (!QSqlDatabase::contains(connectionName))
  {
    auto db =
      QSqlDatabase::addDatabase(
        plugin,
        connectionName);

    db.setHostName(hostName);
    db.setPort(hostPort);
    db.setDatabaseName(databaseName);
    db.setUserName(userName);
    db.setPassword(password);

    if (!db.open())
      return {};
  }

  auto db =
    QSqlDatabase::database(
      connectionName);

  if (!db.isOpen() && !db.open())
    return {};

  return db;
}