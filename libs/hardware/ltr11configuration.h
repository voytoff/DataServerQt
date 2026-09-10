#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace qds
{

struct Ltr11ChannelConfiguration
{
  uint8_t channel = 0;
  uint8_t mode = 0;
  uint8_t range = 0;
};

struct Ltr11Configuration
{
  uint32_t address = 0;
  uint16_t port = 0;
  std::string crateSerial;
  int slot = 0;

  double channelRate = 1000.0;

  std::vector<Ltr11ChannelConfiguration> channels;
};

}