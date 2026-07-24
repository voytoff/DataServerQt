#ifndef TST_DATASOURCE_H
#define TST_DATASOURCE_H

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
  void test_generatorDataSource_periodicCall();

  void test_dataSourceManager_withoutSources();
  void test_dataSourceManager_withFakeSource();

  // TODO перенестив tst_hardware, если тестов hardware станет много
  void test_hardwareFactory_createFake();
  void test_hardwareFactory_unknownType();

};

#endif // TST_DATASOURCE_H
