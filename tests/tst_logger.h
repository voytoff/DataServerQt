#pragma once

#include <QObject>

class tst_logger : public QObject
{
  Q_OBJECT
public:
  tst_logger();
  ~tst_logger() override;

private slots:
  void test_logger_base();
  void test_logger_levels();

};
