#ifndef TST_SIGNALPROCESSOR_H
#define TST_SIGNALPROCESSOR_H

#include <QObject>

class tst_signalprocessor : public QObject
{
  Q_OBJECT
public:
  tst_signalprocessor();
  ~tst_signalprocessor() override;

private slots:
  void test_signalprocessor_base();

};

#endif // TST_SIGNALPROCESSOR_H
