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

  void test_dataServer_udpSubscription();
  void test_dataServer_failStart_moduleType();
  void test_dataServer_failSubscribe_invalidSignalId();
  void test_dataServer_failSubscribe_duplicateSignalId();
  void test_dataServer_failSubscribe_invalidRate();
  void test_dataServer_failSubscribe_emptyList();
  void test_dataServer_failSubscribe_tooManySignals();

  void test_dataServer_unsubscribe_ok();
  void test_dataServer_unsubscribe_invalidId();

  void test_dataServer_start_stop();
  void test_dataServer_start_after_failed_start();

};

