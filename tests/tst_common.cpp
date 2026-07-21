#include "tst_common.h"
#include <qudpsocket.h>

#include "datatypes.h"
#include "systemconfiguration.h"

test_common::test_common() { }
test_common::~test_common() = default;

void test_common::test_configuration_moduleTags()
{
  using namespace qds;

  SystemConfiguration cfg;

  ModuleInfo m0;
  m0.id = {0};
  cfg.addModule(m0);

  TagInfo t0;
  t0.tag = {0};
  t0.module = {0};
  cfg.addTag(t0);

  TagInfo t5;
  t5.tag = {5};
  t5.module = {0};
  cfg.addTag(t5);

  const auto& tags = cfg.moduleTags(ModuleId{0});

  QCOMPARE(tags.size(), 2u);
  QCOMPARE(tags[0].value, 0u);
  QCOMPARE(tags[1].value, 5u);
}
