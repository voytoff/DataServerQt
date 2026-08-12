#pragma once

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

  void test_datasource_layout_contains();
  void test_datasource_layout_reference();
  void test_datasource_layout_unknownModule();
  void test_datasource_layout_rebuild();

};