#pragma once

#include "ilcardmodule.h"
#include "moduleconfiguration.h"

namespace qds
{

class Ltr11Module : public ILCardModule
{
public:
  explicit Ltr11Module(
    const ModuleConfiguration& configuration);

  ~Ltr11Module() noexcept override;

  bool start() noexcept override;
  void stop() noexcept override;

  bool read(std::span<double> values) noexcept override;

private:
  bool configure() noexcept;
  bool configureChannels() noexcept;
  bool configureFrequency() noexcept;

private:
  ModuleConfiguration m_configuration;

  TLTR11 m_hltr11{};

  uint32_t m_channelCount = 0;

  uint32_t m_recvDataCount = 32;

  std::vector<DWORD> m_rbuf;
  std::vector<double> m_data;
};

}
