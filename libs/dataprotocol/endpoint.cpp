#include "endpoint.h"
#include <format>

namespace qds
{

std::string Endpoint::toString() const
{
  return std::format("{}:{}", address, port);
}

}