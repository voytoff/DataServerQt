#pragma once

#include "endpoint.h"
#include <cstddef>
#include <span>

namespace qds
{

class ISender
{
public:
  virtual ~ISender() = default;

  virtual bool send(
    const Endpoint& endpoint,
    std::span<const std::byte> data) = 0;
};

}