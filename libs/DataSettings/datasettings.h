#pragma once

namespace qds
{

struct TableSettings
{
  int decimals = 6;
};

enum class ChartMarkerMode
{
  Interpolated,
  NearestSample
};

struct ChartSettings
{
  int lineWidth = 1;

  ChartMarkerMode markerMode =
    ChartMarkerMode::Interpolated;

  int decimals = 3;
};

}