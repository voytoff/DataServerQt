#include "calibration.h"

namespace qds
{

bool Calibration::buildSegments()
{
  segments.clear();

  if (points.size() < 2)
    return false;

  segments.reserve(points.size() - 1);

  for (std::size_t i = 0; i + 1 < points.size(); ++i)
  {
    const auto& p0 = points[i];
    const auto& p1 = points[i + 1];

    if (p1.x <= p0.x)
      return false;

    CalibrationSegment segment;

    segment.x0 = p0.x;
    segment.y0 = p0.y;
    segment.k =
      (p1.y - p0.y) /
      (p1.x - p0.x);

    segments.push_back(segment);
  }

  return true;
}

bool Calibration::apply(
  double x,
  double& y) const
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
  {
    // x левее диапазона.
    it = segments.begin();
  }
  else
  {
    --it;
  }

  y =
    it->y0 +
    (x - it->x0) * it->k;

  return true;
}

}