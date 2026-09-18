#pragma once

#include <QObject>

class tst_hardware : public QObject
{
  Q_OBJECT
public:
  tst_hardware();
  ~tst_hardware() override;

private slots:
  void test_fakeLCardModule_base();
  void test_lCardDataSource();
  void test_lCardDataSource_data_integrity();

  void test_ltr11configurationbuilder();

  void test_acquire_returns_last_frame();
  void test_lCardDataSource_fake_push_archive();

};
