#ifndef SCHEDULERCLOCK_H
#define SCHEDULERCLOCK_H

#include "datatypes.h"

namespace qds
{

class SchedulerClock
{
public:

  Timestamp now() const;

  WallClockTime wallNow() const;

  //FrameNumber nextFrame();

private:

  uint64_t m_frameCounter = 0;
};

}

#endif // SCHEDULERCLOCK_H
