#pragma once

#include <QObject>

class tst_dataserver : public QObject
{
  Q_OBJECT
public:
  tst_dataserver();
  ~tst_dataserver() override;

private slots:

  void test_dataServer_publish_archive_pipeline();

  void test_systemBuilder_success();
  void test_systemBuilder_buildRuntime();

  void test_systemBuilder_failErrorFormula();
  void test_systemBuilder_failDataSourceManager();
  void test_systemBuilder_pipeline();

  ///void test_dataServer_udpSubscription();
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
  void test_dataServer_failStart_invalidUdpPort();
  void test_dataServer_subscriptionId_after_restart();
  void test_SystemBuilder_buildAfterFailure();

  void test_dataEngine_process_without_initialize();
  void test_dataEngine_process_success();
  void test_dataServer_stop_on_dataSourceFailure();
  void test_SystemBuilder_failedThenSuccess();

  void test_dataServer_start_twice();
  void test_dataServer_stop_before_start();
  void test_dataServer_udp_pipeline();

  void test_dataServer_failModule();

};

