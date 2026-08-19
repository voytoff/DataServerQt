#pragma once

#include <QObject>

class tst_dataserver : public QObject
{
  Q_OBJECT
public:
  tst_dataserver();
  ~tst_dataserver() override;

private slots:
  void test_systemBuilder_success();
  void test_systemBuilder_process();

  void test_systemBuilder_failErrorFormula();
  void test_systemBuilder_failDataSourceManager();
  void test_systemBuilder_cycle();

};

