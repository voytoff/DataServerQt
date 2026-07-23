#ifndef TST_HARDWARE_H
#define TST_HARDWARE_H

#include <QObject>

class tst_hardware : public QObject
{
  Q_OBJECT
public:
  tst_hardware();
  ~tst_hardware() override;

private slots:
  void test_hardware_schedulerPipeline();

};

#endif // TST_HARDWARE_H
