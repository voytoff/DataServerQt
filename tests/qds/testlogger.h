#pragma once

#include <string>
#include <vector>
#include "ilogger.h"

namespace qds
{

class TestLogger : public ILogger
{
public:
  std::vector<std::string> errors;

  bool debug(std::string_view message) override;
  bool info(std::string_view message) override;
  bool warning(std::string_view message) override;
  bool error(std::string_view message) override;

};

}