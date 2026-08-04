#pragma once

#include <QObject>

class tst_scheduler : public QObject
{
  Q_OBJECT
public:
  tst_scheduler();
  ~tst_scheduler() override;

private slots:
  void test_scheduler_base();
};

