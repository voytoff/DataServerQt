#include "calibration.h"

#include <algorithm>
#include <cmath>

namespace qds
{

bool Calibration::buildSegments()
{
  segments.clear();

  if (kind != CalibrationKind::Table)
    return false;

  if (points.size() < 2)
    return false;

  segments.reserve(points.size() - 1);

  for (std::size_t i = 0; i + 1 < points.size(); ++i)
  {
    const auto& p0 = points[i];
    const auto& p1 = points[i + 1];

    if (!std::isfinite(p0.x) ||
        !std::isfinite(p0.y) ||
        !std::isfinite(p1.x) ||
        !std::isfinite(p1.y) ||
        p1.x <= p0.x)
    {
      segments.clear();
      return false;
    }

    CalibrationSegment segment;

    segment.x0 = p0.x;
    segment.y0 = p0.y;
    segment.k =
      (p1.y - p0.y) /
      (p1.x - p0.x);

    if (!std::isfinite(segment.k))
    {
      segments.clear();
      return false;
    }

    segments.push_back(segment);
  }

  return true;
}

bool Calibration::apply(
  double x,
  double& y) const
{
  if (!std::isfinite(x))
    return false;

  double result = 0.0;

  switch (kind)
  {
  case CalibrationKind::Table:
  {
    if (segments.empty())
      return false;

    auto it =
      std::upper_bound(
        segments.begin(),
        segments.end(),
        x,
        [](double value,
           const CalibrationSegment& segment)
        {
          return value < segment.x0;
        });

    if (it == segments.begin())
      it = segments.begin();
    else
      --it;

    result =
      it->y0 +
      (x - it->x0) * it->k;

    break;
  }

  case CalibrationKind::Polynomial:
  {
    if (coefficients.empty())
      return false;

    // Схема Горнера.
    for (auto it = coefficients.rbegin();
         it != coefficients.rend(); ++it)
    {
      result = result * x + *it;
    }

    break;
  }

  default:
    return false;
  }

  if (!std::isfinite(result))
    return false;

  y = result;
  return true;
}

}