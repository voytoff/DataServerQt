#include "systemconfiguration.h"
#include "dataserver.h"

#include <QTimer>
#include <QCoreApplication>

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  qds::SystemConfiguration config;
  qds::CrateInfo ci{0};
  config.addCrate(ci);

  qds::ModuleInfo mi{.id = {0}, .crate = ci.id, .type = qds::ModuleType::Fake};
  config.addModule(mi);

  std::vector<qds::TagId> tags{{0}, {1}, {2}};

  for (int i = 0; i < tags.size(); i++)
  {
    const auto &tag = tags[i];
    qds::TagInfo ti{.tag = tag, .module = mi.id, .channel = {tag.value}};
    config.addTag(ti);
  }

  DataServer server(config);

  if (!server.start())
    return -1;

  return app.exec();
}