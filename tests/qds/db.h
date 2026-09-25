#pragma once

#include <ranges>
#include <vector>
#include <string>
#include <QSqlDatabase>
#include <QSqlQuery>

#if defined(__APPLE__)
const QString plugin = "QMYSQL";
#else // Linux & Windows
const QString plugin = "QMARIADB";
#endif
const QString hostName = "127.0.0.1";
const int hostPort = 3306;
const QString databaseName = "dataserver";
const QString userName = "root";
const QString password = "1234";

using namespace Qt::StringLiterals;
const QString connectionName = u"sql_default_connection"_s;

/*
 * MAC OS
 * bash

brew install qt-mysql

brew install mysql ninja

mkdir build-sqldrivers
cd build-sqldrivers

~/Qt/6.11.1/macos/bin/qt-cmake -G Ninja \
  ~/Qt/6.11.1/Src/qtbase/src/plugins/sqldrivers \
  -DCMAKE_INSTALL_PREFIX=~/Qt/6.11.1/macos \
  -DMySQL_ROOT="/opt/homebrew/opt/mysql" \
  -DFEATURE_sql_sqlite=OFF

sed -i '' 's/-arch x86_64/-arch arm64/g' build.ninja

cmake --build .
cmake --install .

 */
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

inline QSqlQuery selectByIds(const QString &sql, const std::vector<uint32_t>& ids) {
  if (ids.empty()) {
    // Запрос IN () вызовет синтаксическую ошибку в MySQL/MariaDB
    return QSqlQuery(get_db());
  }

  // C++23: генерируем строку из знаков "?" через диапазоны
  //auto placeholders_view = std::views::repeat("?") | std::views::take(ids.size());
  QStringList placeholders;
  placeholders.reserve(ids.size());
  //for (auto p : placeholders_view)
  for (int m = 0; m < ids.size(); ++m)
    placeholders.append(/*p*/"?");

  // Формируем безопасный SQL-запрос
  QString queryString = QString(sql)
                          .arg(placeholders.join(','));

  QSqlQuery query(get_db());
  query.prepare(queryString);

  // Последовательно привязываем int-значения
  for (int id : ids)
    query.addBindValue(id);

  return query;
}
