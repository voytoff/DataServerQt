#pragma once

#include <QObject>

class tst_ltr11module : public QObject
{
  Q_OBJECT
public:
  tst_ltr11module();
  ~tst_ltr11module() override;



private slots:
  void test_LCardDataSource_update_frequency();
  void test_Ltr11Module_base();
  void test_Ltr11Module_without_print();
  void test_LCardDataSource_base();

  void test_ltr11configurationvalidator();
  void test_Ltr11ConfigurationFactory();

};
