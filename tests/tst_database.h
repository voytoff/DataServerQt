#pragma once

#include <QObject>

class tst_database : public QObject
{
  Q_OBJECT
public:
  tst_database();
  ~tst_database() override;

  void test_database_failLoading();

private slots:
  void test_database_loadConfiguration();
  //void test_database_loadCalibrations();
  void test_database_pipeline();

  void test_database_archive();

  void test_archiveReader_open();
  void test_archiveReader_read();
  void test_archiveReader_readFrame();

  void test_archiveReader_readRecord();
  void test_archiveReader_findRecord();
  void test_archiveReader_missingFrames();
  void test_archiveSignalSource();
  void test_archiveSignalSource_readPlot();
  void test_archiveSignalSource_cancel();

  void test_publisher();
  void test_publisher_raw_calculated();

};

