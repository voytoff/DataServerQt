#pragma once

#include "ilcardmodule.h"

#include "ltr11api.h"
#include "ltr11configuration.h"

#include <cstdint>
#include <span>
#include <vector>

namespace qds
{

class Ltr11Module final : public ILCardModule
{
public:
  explicit Ltr11Module(
    const Ltr11Configuration& configuration);
  ~Ltr11Module() noexcept override;

  bool start() noexcept override;
  void stop() noexcept override;

  bool read(std::span<double> values) noexcept override;

private:
  bool configureConnection() noexcept;
  bool configureChannels() noexcept;
  bool configureFrequency() noexcept;

  bool processReceivedData(
    const DWORD* data,
    int count,
    std::span<double> values) noexcept;

private:
  Ltr11Configuration m_configuration;

  TLTR11 m_hltr11{};

  std::size_t m_channelCount = 0;

  // Number of raw DWORDs requested from LTR11_Recv().
  uint32_t m_recvDataCount = 32;

  std::vector<DWORD> m_recvBuffer;
  std::vector<double> m_data;

  bool m_initialized = false;
  bool m_opened = false;
  bool m_started = false;
};

} // namespace qds