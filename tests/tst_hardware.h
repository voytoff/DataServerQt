#pragma once

#include <QObject>

class tst_hardware : public QObject
{
  Q_OBJECT
public:
  tst_hardware();
  ~tst_hardware() override;

private slots:
  //void test_hardware_schedulerPipeline();
  void test_fakeLCardModule_base();
  void test_lCardDataSource();
  void test_lCardDataSource_data_integrity();

};