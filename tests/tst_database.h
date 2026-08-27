#pragma once

#include <QObject>

class tst_database : public QObject
{
  Q_OBJECT
public:
  tst_database();
  ~tst_database() override;

private slots:
  void test_database_loadConfiguration();

};

