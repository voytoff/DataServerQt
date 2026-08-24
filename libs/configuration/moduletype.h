#pragma once

namespace qds
{

enum class ModuleType
{
  Unknown = 0,
  Fake,
  Failing,
  FailOnce,
  Test,
  LCard
};

}