#pragma once

namespace qds
{

enum class ModuleType
{
  Unknown = 0,
  Fake, FakeOnce,
  Fail,
  Test,
  LCard
};

}