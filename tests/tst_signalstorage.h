#ifndef TST_SIGNALSTORAGE_H
#define TST_SIGNALSTORAGE_H

#include <QObject>

class tst_signalstorage : public QObject
{
  Q_OBJECT
public:
  tst_signalstorage();
  ~tst_signalstorage() override;

private slots:
  void test_signalstorage_base();
  void test_buffer_manager();
  void test_raw_memory();

};

#endif // TST_SIGNALSTORAGE_H
