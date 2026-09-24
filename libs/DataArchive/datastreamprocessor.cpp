#include "datastreamprocessor.h"

namespace qds
{

DataStreamProcessor::DataStreamProcessor(
  DataStreamReader &reader,
  FrameAssembler &assembler,
  SignalProcessor &signalProcessor,
  BufferManager &buffers,
  IArchiveWriter &archive,
  ILogger &logger)
  : m_reader(reader)
  , m_assembler(assembler)
  , m_signalProcessor(signalProcessor)
  , m_buffers(buffers)
  , m_archive(archive)
  , m_logger(logger)
{
}

bool DataStreamProcessor::process(
  const DataStreamEvent& event)
{
  if (!m_reader.process(
        event,
        m_frames))
  {
    return false;
  }

  for (const DataStreamFrame& streamFrame :
       m_frames)
  {
    auto assembled =
      m_assembler.push(streamFrame);

    if (!assembled.has_value())
      continue;

    Frame& frame =
      assembled.value();

    if (!m_signalProcessor.process(
          frame.raw(),
          frame.calculated()))
    {
      return false;
    }

    m_buffers.publish(frame);

    if (!m_archive.write(frame))
      m_logger.error("Archive write failed");
  }

  return true;
}

}