#pragma once

#include <QObject>

class tst_configuration : public QObject
{
  Q_OBJECT
public:
  tst_configuration();
  ~tst_configuration() override;

private slots:
  void test_configuration_crates();
  void test_configuration_modules();
  void test_configuration_tags();
  void test_configuration_parse_module_ids();
  void test_configuration_moduleTags();
  void test_configuration_moduleChannelCount();
  void test_configuration_signalDefinitions();
  void test_configuration_empty();
  void test_configuration_sparseTagIds();
  void test_configuration_duplicateTagIds();
  void test_configuration_duplicateSignalDefinitions();
  void test_configuration_failTagModule();
  void test_configuration_sparseSignalDefinitionIds();
  void test_configuration_orderIds();

};

