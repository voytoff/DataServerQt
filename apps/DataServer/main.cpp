#include "fakeschedulerclock.h"
#include "systemconfiguration.h"
#include "dataserver.h"
#include "testdatasource.h"
#include "testpublisher.h"
#include "testarchivewriter.h"
#include "udpsender.h"

#include <QCoreApplication>

static qds::SystemConfiguration createTestConfig_calculate(qds::ModuleType type = qds::ModuleType::Unknown)
{
  using namespace qds;
  SystemConfiguration cfg;

  ModuleInfo m{.id = {0}, .type = type};
  cfg.addModule(m);

  cfg.addTag({.tag = {0}, .module = {0}, .channel = {0}});
  cfg.addTag({.tag = {1}, .module = {0}, .channel = {1}});

  cfg.addSignalDefinition({.id = {0}, .name = "Raw0", .kind = SignalKind::Raw, .source = {0}, .archiveFrequency = 1000});
  cfg.addSignalDefinition({.id = {1}, .name = "Raw1", .kind = SignalKind::Raw, .source = {1}, .archiveFrequency = 100});

  cfg.addSignalDefinition({.id = {17}, .name = "A", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .formula = "Raw0", .dependencies = {{0}}}); // Raw0

  cfg.addSignalDefinition({.id = {4}, .name = "B", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {1}, .formula = "Raw1", .dependencies = {{1}}});  // Raw1

  cfg.addSignalDefinition({.id = {23}, .name = "C", .kind = SignalKind::Calculated, .archiveFrequency = 10, .formulaId = {2}, .formula = "A + B", .dependencies = {{17}, {4}}}); // A & B

  return cfg;
}

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  qds::SystemConfiguration cfg = createTestConfig_calculate(qds::ModuleType::Test);
  qds::DataSourceFactory factory;

  if (!factory.registerType(
    qds::ModuleType::Test,
    [](const qds::ModuleConfiguration& cfg)
    {
      return std::make_unique<qds::TestDataSource>(
        cfg.settings);
    }))
      return -1;

  qds::TestArchiveWriter archive;
  qds::TestPublisher publisher;
  qds::FakeSchedulerClock clock;
  qds::UdpSender sender;

  qds::DataServer server(
    cfg,
    factory,
    archive,
    clock,
    sender);

  if (!server.start())
    return -1;

  return app.exec();
}
