#pragma once

#include <QObject>

class tst_dataarchive : public QObject
{
  Q_OBJECT
public:
  tst_dataarchive();
  ~tst_dataarchive() override;

  void test_DataBlockQueue_deque();
  void test_DataBlockQueue_waitPop();
  void test_DataBlockQueue_stop();
  void test_DataBlockQueue_stop_drains_queue();

  void test_DataBlockQueue_firstFrameIndex();
  void test_DataBlockQueue_firstFrameIndex_two_blocks();
  void test_DataBlockQueue_firstFrameIndex_three_blocks();


private slots:
  void test_archivewriter_create_open();
  void test_archivewriter_writeOneRecord();
  void test_archivewriter_writeSomeRecords();
  void test_archivewriter_badChannelCount();
  void test_archivewriter_writeAfterClose();
  void test_archivewriter_doubleClose();

  void test_dataFileHeader_headerSize();
  void test_dataFileHeader_base();
  void test_archiveFile_emptyFile();
  void test_archiveFile_badMagic();
  void test_archiveFile_invalidVersion();
  void test_archiveFile_invalidRecordSize();
  void test_archiveFile_invalidHeaderSize();
  void test_archiveFile_badChannelCount();
  void test_archiveFile_reCreates();
  void test_archiveFile_saveHeader();
  void test_archiveFile_saveHeaderInReadMode();
  void test_archiveFile_seekToZerro();
  void test_archiveFile_seekToDataAfterHeader();
  void test_archiveFile_saveZerroArray();
  void test_archiveFile_checkFileSize();
  void test_archiveFile_saveHeaderNotChangePosition();

  void test_archiveDescription_writeRawSignals();
  void test_archiveDescription_writeCalculatedSignals();
  void test_archiveDescription_writeMultipleFiles();

  void test_archiveDescription_archiveDescriptionBuilder();

  //void test_DataBlockQueue_deque();
  //void test_DataBlockQueue_waitPop();
  //void test_DataBlockQueue_stop();
  //void test_DataBlockQueue_stop_drains_queue();

  //void test_DataBlockQueue_firstFrameIndex();
  //void test_DataBlockQueue_firstFrameIndex_two_blocks();
  //void test_DataBlockQueue_firstFrameIndex_three_blocks();

  void test_DataStreamEventSink_base();
  void test_DataStreamEvent_base();
  void test_DataBlockQueue_anchor_only();
  void test_DataBlockQueue_anchor_block();
  void test_DataBlockQueue_three_blocks();
  void test_LCardDataSource_stream_events();
  void test_DataBlockQueue_stream_events();
  void test_DataStreamTime_base();
  void test_DataStreamTime_frameRate_0();
  void test_DataStreamTime_restart();
  void test_DataStreamTime_frameRate_44100();
  void test_DataStreamReader_block();
  void test_DataStreamReader_block_before_anchor();
  void test_DataStreamReader_bad_block();

  void test_FrameAssembler_wait_for_all_modules();
  void test_FrameAssembler_allow_incomplete();
  void test_DataStreamProcessor_wait_for_all();

  void test_DataStreamProcessor_livetime();
  void test_DataStreamProcessor_block_with_multiple_frames();

  void test_DataStreamWorker_base();
  void test_LCardDataSource_to_archive();
  void test_LCardDataSource_some_modules_to_archive();
};
