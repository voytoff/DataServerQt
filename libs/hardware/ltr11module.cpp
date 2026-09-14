#include "ltr11module.h"

#include "ltr11api.h"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace qds
{

namespace
{

constexpr std::size_t RecvBlockFrameCount = 10;
constexpr DWORD RecvTimeoutMs = 100;

}

class Ltr11Module::Impl
{
public:
  explicit Impl(
    const Ltr11Configuration& configuration);

  bool start() noexcept;
  void stop() noexcept;
  std::size_t blockFrameCapacity() const noexcept;
  std::size_t readBlock(std::span<double> values) noexcept;

private:
  bool configureConnection() noexcept;
  bool configureChannels() noexcept;
  bool configureFrequency() noexcept;

private:
  Ltr11Configuration m_configuration;

  TLTR11 m_hltr11{};

  std::size_t m_channelCount = 0;
  uint32_t m_recvDataCount = 0;

  std::vector<DWORD> m_recvBuffer;
  std::vector<double> m_data;

  bool m_initialized = false;
  bool m_opened = false;
  bool m_started = false;
};

Ltr11Module::Ltr11Module(
  const Ltr11Configuration& configuration)
  : m_impl(std::make_unique<Impl>(configuration))
{
}

Ltr11Module::~Ltr11Module() noexcept = default;

bool Ltr11Module::start() noexcept
{
  return m_impl->start();
}

void Ltr11Module::stop() noexcept
{
  m_impl->stop();
}

std::size_t Ltr11Module::blockFrameCapacity() const noexcept
{
  return m_impl->blockFrameCapacity();
}

std::size_t Ltr11Module::readBlock(std::span<double> values) noexcept
{
  return m_impl->readBlock(values);
}

Ltr11Module::Impl::Impl(
  const Ltr11Configuration& configuration)
  : m_configuration(configuration)
{
  m_channelCount =
    m_configuration.channels.size();

  /*
   * One frame contains one value for each logical channel.
   *
   * Receive up to 10 complete frames in one block.
   */
  m_recvDataCount =
    static_cast<uint32_t>(
      m_channelCount * RecvBlockFrameCount);

  m_recvBuffer.resize(m_recvDataCount);
  /*
   * ProcessData() writes converted values here.
   *
   * The buffer may contain several complete frames.
   */
  m_data.resize(m_recvDataCount);

  /*
  const double blockDurationMs =
    1000.0 *
    static_cast<double>(RecvBlockFrameCount) /
    m_configuration.channelRate;

  const DWORD timeout =
    static_cast<DWORD>(blockDurationMs) + 100;
*/
}


bool Ltr11Module::Impl::start() noexcept
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
   * передаем настройки в модуль.
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


void Ltr11Module::Impl::stop() noexcept
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

std::size_t Ltr11Module::Impl::blockFrameCapacity() const noexcept
{
  return RecvBlockFrameCount;
}

std::size_t Ltr11Module::Impl::readBlock(
  std::span<double> values) noexcept
{
  if (!m_started)
    return 0;

  if (m_channelCount == 0)
    return 0;

  const std::size_t requiredSize =
    m_channelCount * RecvBlockFrameCount;

  if (values.size() < requiredSize)
    return 0;

  const int received = LTR11_Recv(
    &m_hltr11,
    m_recvBuffer.data(),
    nullptr,
    static_cast<DWORD>(m_recvBuffer.size()),
    RecvTimeoutMs);

  if (received <= 0)
    return 0;

  int processedCount = received;

  const int err = LTR11_ProcessData(
    &m_hltr11,
    m_recvBuffer.data(),
    m_data.data(),
    &processedCount,
    TRUE,
    TRUE);

  if (err != 0)
    return 0;

  if (processedCount <= 0)
    return 0;

  const std::size_t frameCount =
    static_cast<std::size_t>(processedCount) /
    m_channelCount;

  if (frameCount == 0)
    return 0;

  const std::size_t valueCount =
    frameCount * m_channelCount;

  std::copy_n(
    m_data.data(),
    valueCount,
    values.data());

  return frameCount;
}

bool Ltr11Module::Impl::configureConnection() noexcept
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

bool Ltr11Module::Impl::configureChannels() noexcept
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

bool Ltr11Module::Impl::configureFrequency() noexcept
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