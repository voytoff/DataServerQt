#include "emptysignalprocessor.h"

namespace qds
{

bool EmptySignalProcessor::process(Frame &frame)
{
  applyCalibrations(frame);
  evaluateFormulas(frame);

  return true;
}

void EmptySignalProcessor::applyCalibrations(Frame &frame)
{

}

void EmptySignalProcessor::evaluateFormulas(Frame &frame)
{
}

}