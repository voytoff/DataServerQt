#pragma once

namespace qds
{

enum class CrateType
{
  Unknown = 0,

  LTR_U_8 = 1,
  LTR_U_16 = 2,
  LTR_U_1 = 3,

  LTR_EU_8 = 4,
  LTR_EU_16 = 5,
  LTR_EU_2 = 6,

  LTR_E_7 = 7,
  LTR_E_15 = 8,

  LTR_CU_1 = 9,
  LTR_CEU_1 = 10
};

}