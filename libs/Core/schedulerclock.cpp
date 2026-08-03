#include "schedulerclock.h"

namespace qds
{

//FrameNumber SchedulerClock::nextFrame()
//{
//}

Timestamp SchedulerClock::now() const
{
  return {0};
}

WallClockTime SchedulerClock::wallNow() const
{
  return {0};
}

}