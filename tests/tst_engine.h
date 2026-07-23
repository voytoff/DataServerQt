#ifndef TST_ENGINE_H
#define TST_ENGINE_H

#include <QObject>

class tst_engine : public QObject
{
  Q_OBJECT
public:
  tst_engine();
  ~tst_engine() override;

private slots:
  void test_dataEngine_withoutSources();
  void test_dataEngine_withFakeSource();
  void test_dataEngine_stepWithoutStart();
  void test_dataEngine_callSomeStops();
  void test_dataEngine_restart();
  void test_dataEngine_storagePipeline();
  // здесь проверяется буквально вся система
  void test_dataEngine_completePipeline();
};

#endif // TST_ENGINE_H
