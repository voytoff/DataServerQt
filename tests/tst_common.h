#ifndef TST_COMMON_H
#define TST_COMMON_H

#include <QObject>
#include <QTest>

class test_common : public QObject
{
  Q_OBJECT

public:
  test_common();
  ~test_common() override;

private slots:
  void test_configuration_moduleTags();

};

#endif // TST_COMMON_H
