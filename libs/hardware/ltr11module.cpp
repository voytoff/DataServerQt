#include "ltr11module.h"

#include <algorithm>

namespace qds
{

namespace
{

constexpr std::size_t RecvFrameCount = 8;
constexpr std::size_t DefaultRecvDataCount = 32;
constexpr WORD DefaultPort = LTRD_PORT_DEFAULT;

}

Ltr11Module::Ltr11Module(const Ltr11Configuration& configuration)
  : m_configuration(configuration)
{
  m_channelCount = m_configuration.channels.size();

  /*
     * LTR11_Recv() works with raw DWORD words.
     *
     * 32 is a reasonable minimum block size.
     * If the number of logical channels is larger,
     * make the buffer large enough to contain at least
     * one complete frame.
     */
  m_recvDataCount = std::max(
    DefaultRecvDataCount,
    m_channelCount * RecvFrameCount);

  m_recvBuffer.resize(m_recvDataCount);

  /*
     * ProcessData() writes converted values here.
     *
     * The buffer may contain several complete frames.
     */
  m_data.resize(m_recvDataCount);
}

Ltr11Module::~Ltr11Module() noexcept
{
  stop();
}


bool Ltr11Module::start() noexcept
{
  if (m_started)
    return true;

  /*
     * Initialize descriptor.
     */
  if (!m_initialized)
  {
    if (LTR11_Init(&m_hltr11) != 0)
      return false;

    m_initialized = true;
  }

  /*
     * Open connection to LTRD/module.
     */
  if (!configureConnection())
  {
    if (m_initialized)
    {
      LTR11_Close(&m_hltr11);
      m_opened = false;
      m_initialized = false;
    }

    return false;
  }

  /*
     * Configure logical channels.
     */
  if (!configureChannels())
  {
    LTR11_Close(&m_hltr11);
    m_opened = false;
    m_initialized = false;

    return false;
  }

  /*
     * Configure ADC frequency.
     */
  if (!configureFrequency())
  {
    LTR11_Close(&m_hltr11);
    m_opened = false;
    m_initialized = false;

    return false;
  }

  /*
     * For now these modes are fixed.
     */
  m_hltr11.StartADCMode = LTR11_STARTADCMODE_INT;
  m_hltr11.InpMode      = LTR11_INPMODE_INT;
  m_hltr11.ADCMode      = LTR11_ADCMODE_ACQ;

  /*
     * Send ADC configuration to module.
     */
  if (LTR11_SetADC(&m_hltr11) != 0)
  {
    LTR11_Close(&m_hltr11);
    m_opened = false;
    m_initialized = false;

    return false;
  }

  /*
     * Start data acquisition.
     */
  if (LTR11_Start(&m_hltr11) != 0)
  {
    LTR11_Close(&m_hltr11);
    m_opened = false;
    m_initialized = false;

    return false;
  }

  m_started = true;

  return true;
}


void Ltr11Module::stop() noexcept
{
  if (m_started)
  {
    LTR11_Stop(&m_hltr11);
    m_started = false;
  }

  if (m_opened)
  {
    LTR11_Close(&m_hltr11);
    m_opened = false;
  }

  m_initialized = false;
}


bool Ltr11Module::read(std::span<double> values) noexcept
{
  if (!m_started)
    return false;

  if (values.size() != m_channelCount)
    return false;

  if (values.empty())
    return false;

  /*
     * LTR11_Recv() returns the actual number of received
     * raw DWORDs.
     *
     * It may be smaller than m_recvDataCount because of
     * timeout, therefore we must use the returned value.
     */
  const int received = LTR11_Recv(
    &m_hltr11,
    m_recvBuffer.data(),
    nullptr,
    static_cast<DWORD>(m_recvBuffer.size()),
    100);

  if (received < 0)
    return false;

  if (received == 0)
    return false;

  return processReceivedData(
    m_recvBuffer.data(),
    received,
    values);
}


bool Ltr11Module::processReceivedData(
  const DWORD* data,
  int count,
  std::span<double> values) noexcept
{
  if (data == nullptr)
    return false;

  if (count <= 0)
    return false;

  /*
     * ProcessData() uses size as an input/output parameter.
     *
     * Input:
     *     number of raw words in data
     *
     * Output:
     *     number of processed double values.
     */
  int processedCount = count;

  const int err = LTR11_ProcessData(
    &m_hltr11,
    data,
    m_data.data(),
    &processedCount,
    TRUE,
    TRUE);

  if (err != 0)
    return false;

  if (processedCount <= 0)
    return false;

  /*
     * One frame consists of LChQnt values.
     *
     * We don't require the whole processed buffer to be
     * aligned. If the last frame is incomplete, simply
     * ignore it and use the last complete frame.
     */
  const int frameSize =
    static_cast<int>(m_channelCount);

  if (frameSize <= 0)
    return false;

  const int frameCount =
    processedCount / frameSize;

  if (frameCount <= 0)
    return false;

  const int lastFrameOffset =
    (frameCount - 1) * frameSize;

  std::copy_n(
    m_data.data() + lastFrameOffset,
    frameSize,
    values.data());

  return true;
}


bool Ltr11Module::configureConnection() noexcept
{
  if (m_opened)
    return true;

  const WORD port =
    m_configuration.port != 0
      ? m_configuration.port
      : LTRD_PORT_DEFAULT;

  const int err = LTR11_Open(
    &m_hltr11,
    m_configuration.address,
    port,
    m_configuration.crateSerial.c_str(),
    m_configuration.slot);

  if (err != 0)
    return false;

  m_opened = true;

  if (LTR11_GetConfig(&m_hltr11) != 0)
  {
    LTR11_Close(&m_hltr11);
    m_opened = false;
    return false;
  }

  return true;
}

bool Ltr11Module::configureChannels() noexcept
{
  const auto& channels = m_configuration.channels;

  if (channels.empty())
    return false;

  if (channels.size() > LTR11_MAX_LCHANNEL)
    return false;

  m_channelCount = channels.size();

  m_hltr11.LChQnt =
    static_cast<INT>(m_channelCount);

  for (std::size_t i = 0; i < channels.size(); ++i)
  {
    const auto& channel = channels[i];

    if (channel.channel >= LTR11_MAX_CHANNEL)
      return false;

    if (channel.mode > LTR11_CHMODE_ZERO)
      return false;

    if (channel.range > LTR11_CHRANGE_156MV)
      return false;

    m_hltr11.LChTbl[i] =
      LTR11_CreateLChannel(
        channel.channel,
        channel.mode,
        channel.range);
  }

  return true;
}

bool Ltr11Module::configureFrequency() noexcept
{
  if (m_channelCount == 0)
    return false;

  /*
     * channelRate is the required sampling frequency
     * of ONE logical channel, in Hz.
     *
     * Example:
     *
     *     channelRate = 1000 Hz
     *     channels    = 4
     *
     *     ADC frequency = 4000 Hz
     */
  const double channelRate = m_configuration.channelRate;

  if (channelRate <= 0.0)
    return false;

  const double requiredAdcFreq =
    channelRate *
    static_cast<double>(m_channelCount);

  /*
     * LTR11_FindAdcFreqParams() accepts total ADC frequency
     * in Hz.
     */
  double resultAdcFreq = 0.0;

  const int err = LTR11_FindAdcFreqParams(
    requiredAdcFreq,
    &m_hltr11.ADCRate.prescaler,
    &m_hltr11.ADCRate.divider,
    &resultAdcFreq);

  if (err != 0)
    return false;

  if (resultAdcFreq <= 0.0)
    return false;

  /*
     * Actual frequency of one logical channel.
     *
     * ChRate is stored by LTR11 in kHz.
     */
  m_hltr11.ChRate =
    resultAdcFreq /
    (1000.0 *
     static_cast<double>(m_channelCount));

  return true;
}

} // namespace qds