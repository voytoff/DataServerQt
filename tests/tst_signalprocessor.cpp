#include "tst_signalprocessor.h"
#include "calculationplan.h"
#include "systemconfiguration.h"

tst_signalprocessor::tst_signalprocessor() { }
tst_signalprocessor::~tst_signalprocessor() = default;

void tst_signalprocessor::test_signalprocessor_base()
{
  using namespace qds;
  SystemConfiguration cfg;

  qds::ModuleInfo m{0};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = {0}, .channel = {0}});

  SignalDefinition sd0 {.id = 0, .name = "U000", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 100};
  cfg.addSignalDefinition(sd0);

  SignalDefinition sd1 {.id = 1, .name = "U001", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 10};
  cfg.addSignalDefinition(sd1);

  SignalDefinition sd2 {.id = 2, .name = "D10", .kind = SignalKind::Calculated, .source = {0}, .archiveFrequency = 10, .formulaId = 3};
  cfg.addSignalDefinition(sd2);

  SignalDefinition sd3 {.id = 3, .name = "P100", .kind = SignalKind::Calculated, .source = {0}, .archiveFrequency = 100, .formulaId = 15};
  cfg.addSignalDefinition(sd3);

  CalculationPlan plan;
}

