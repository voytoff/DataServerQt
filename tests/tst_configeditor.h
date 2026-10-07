#pragma once

#include <QObject>

class tst_configeditor : public QObject
{
  Q_OBJECT

public:
  tst_configeditor();
  ~tst_configeditor() override;

private slots:
  void test_configurationRepository_moduleLifecycle();
  void test_configurationRepository_moduleLifecycle_withTags();
  void test_configurationRepository_configModules();

};

