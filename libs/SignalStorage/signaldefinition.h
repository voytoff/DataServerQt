#ifndef SIGNALDEFINITION_H
#define SIGNALDEFINITION_H

#include <string>
#include "signalstorage.h"

namespace qds
{

struct SignalDefinition
{
  SignalId id;

  std::string name;

  std::string unit;


  bool calculated = false;
};

}

#endif // SIGNALDEFINITION_H