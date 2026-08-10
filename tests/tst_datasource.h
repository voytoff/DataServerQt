#pragma once

#include <QObject>

class tst_datasource : public QObject
{
  Q_OBJECT
public:
  tst_datasource();
  ~tst_datasource() override;

private slots:
  void test_generatorDataSource_once();
  void test_generatorDataSource_onceTwoModule();
  //void test_generatorDataSource_periodicCall();

  //void test_dataSourceManager_withoutSources();
  //void test_dataSourceManager_withFakeSource();

  // TODO перенестив tst_hardware, если тестов hardware станет много
  void test_hardwareFactory_createFake();
  void test_hardwareFactory_unknownType();
  void test_dataSourceFactory_registerType_create();
  void test_datasource_layout();
  void test_datasource_layout_someModules();
  void test_datasource_layout_raw_calculated();
  void test_datasource_layout_two_build();

  void test_datasource_manager();
  void test_datasource_fail_datasource();
  void test_datasource_missing_datasource();
  void test_datasource_repeat_initialize();

};