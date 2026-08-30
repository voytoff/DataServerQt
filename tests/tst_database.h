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
  void test_database_loadCalibrations();
  void test_database_failLoading();
  void test_database_pipeline();

  void test_database_archive();

};

