#pragma once

#include <string_view>

namespace qds
{

class ILogger
{
public:
  virtual ~ILogger() = default;

  virtual bool debug(std::string_view message) = 0;
  virtual bool info(std::string_view message) = 0;
  virtual bool warning(std::string_view message) = 0;
  virtual bool error(std::string_view message) = 0;
};

}