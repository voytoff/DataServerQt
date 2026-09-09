#include "networkutils.h"

namespace qds
{

bool parseIpv4Address(
  std::string_view address,
  uint32_t& result) noexcept
{
  uint32_t value = 0;
  uint32_t part = 0;
  int digits = 0;
  int parts = 0;

  for (char ch : address)
  {
    if (ch >= '0' && ch <= '9')
    {
      if (++digits > 3)
        return false;

      part = part * 10 +
             static_cast<uint32_t>(ch - '0');

      if (part > 255)
        return false;
    }
    else if (ch == '.')
    {
      if (digits == 0)
        return false;

      value = (value << 8) | part;

      part = 0;
      digits = 0;
      ++parts;

      if (parts > 3)
        return false;
    }
    else
    {
      return false;
    }
  }

  if (digits == 0 || parts != 3)
    return false;

  value = (value << 8) | part;

  result = value;

  return true;
}

}