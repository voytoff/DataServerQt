// framestartpolicy.h

#pragma once

namespace qds
{

enum class FrameStartPolicy
{
  WaitForAllModules,
  AllowIncomplete
};

}