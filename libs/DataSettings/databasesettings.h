#pragma once

#include <QString>

namespace qds
{

struct DatabaseSettings
{
  QString plugin = "QMARIADB";
  QString host = "127.0.0.1";
  int port = 3306;

  QString name;
  QString user;
  QString password;
};

}