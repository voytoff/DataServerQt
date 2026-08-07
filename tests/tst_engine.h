#pragma once

#include <QObject>

class tst_engine : public QObject
{
  Q_OBJECT
public:
  tst_engine();
  ~tst_engine() override;

private:
  /*
  void test_dataEngine_withoutSources();
  void test_dataEngine_withFakeSource();
  void test_dataEngine_stepWithoutStart();
  void test_dataEngine_callSomeStops();
  void test_dataEngine_restart();
  void test_dataEngine_storagePipeline();
  // здесь проверяется буквально вся система
*/

private slots:
  void test_dataEngine_simple_pipeline();
  void test_dataEngine_simple_runtime();
  void test_dataEngine_FailingDataSource();
  void test_dataEngine_FailingCalculationProcessor();
  void test_dataEngine_FailingArchiveWriter();

};