#pragma once

#include <QJsonValue>

#include <cmath>
#include <limits>

namespace qds
{

inline bool jsonInteger(
  const QJsonValue& value,
  int minimum,
  int maximum,
  int& result) noexcept
{
  if (!value.isDouble())
    return false;

  const double number = value.toDouble();

  if (!std::isfinite(number) ||
      number < minimum ||
      number > maximum ||
      std::trunc(number) != number)
  {
    return false;
  }

  result = static_cast<int>(number);
  return true;
}

}