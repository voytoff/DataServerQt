#pragma once

#include "buffermanager.h"
#include "datastreamevent.h"
#include "datastreamreader.h"
#include "frameassembler.h"
#include "iarchivewriter.h"
#include "ilogger.h"
#include "signalprocessor.h"

#include <vector>

namespace qds
{

class DataStreamProcessor
{
public:

  DataStreamProcessor(
    DataStreamReader& reader,
    FrameAssembler& assembler,
    SignalProcessor& signalProcessor,
    BufferManager& buffers,
    IArchiveWriter& archive,
    ILogger& logger);

  [[nodiscard]]
  bool process(
    const DataStreamEvent& event);

private:

  DataStreamReader& m_reader;
  FrameAssembler& m_assembler;
  SignalProcessor& m_signalProcessor;

  BufferManager& m_buffers;
  IArchiveWriter& m_archive;
  ILogger& m_logger;

  std::vector<DataStreamFrame> m_frames;
};

}