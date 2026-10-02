#pragma once

#include <cstddef>

namespace qds
{

enum class LCardReadStatus
{
  Data,
  NoData,
  Error
};

struct LCardReadResult
{
  LCardReadStatus status =
    LCardReadStatus::Error;

  std::size_t frameCount = 0;
};

}