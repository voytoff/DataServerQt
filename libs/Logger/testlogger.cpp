#include "testlogger.h"

namespace  qds
{

bool TestLogger::debug(std::string_view message)
{
  return true;
}

bool TestLogger::info(std::string_view message)
{
  return true;
}

bool TestLogger::warning(std::string_view message)
{
  return true;
}

bool TestLogger::error(std::string_view message)
{
  errors.emplace_back(message);
  return true;
}

}
