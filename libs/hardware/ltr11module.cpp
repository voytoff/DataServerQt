#include "ltr11module.h"

namespace qds
{

bool Ltr11Module::read(std::span<double> values) noexcept
{
  if (values.size() != m_channelCount)
    return false;

  INT size = m_recvDataCount;

  const INT err = LTR_Recv(
    &m_hltr11.Channel,
    m_rbuf.data(),
    nullptr,
    m_recvDataCount,
    m_recvTimeout);

  if (err != m_recvDataCount)
    return false;

  if (LTR11_ProcessData(
        &m_hltr11,
        m_rbuf.data(),
        m_data.data(),
        &size,
        TRUE,
        TRUE) != LTR_OK)
  {
    return false;
  }

  if (size < static_cast<INT>(m_channelCount))
    return false;

  const INT frameSize = static_cast<INT>(m_channelCount);

  if (size % frameSize != 0)
    return false;

  const auto* lastFrame =
    m_data.data() + size - frameSize;

  std::ranges::copy(
    std::span(lastFrame, frameSize),
    values.begin());

  return true;
}

}
