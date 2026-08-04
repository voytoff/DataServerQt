#include <qtestcase.h>
#include "tst_signalprocessor.h"
#include "calculationplan.h"
#include "calculationplanbuilder.h"
#include "systemconfiguration.h"
#include "testsrv.h"

tst_signalprocessor::tst_signalprocessor() { }
tst_signalprocessor::~tst_signalprocessor() = default;

void tst_signalprocessor::test_calculation_plan()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig01();

  CalculationPlan plan;

  CalculationPlanBuilder builder;

  QVERIFY(builder.build(cfg, plan));

  QCOMPARE(plan.size(), 2);
}

void tst_signalprocessor::test_calculation_order()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig02();

  CalculationPlan plan;

  CalculationPlanBuilder builder;

  QVERIFY(builder.build(cfg, plan));

  QCOMPARE(plan.size(), 3);

  const auto &order = plan.order();
  auto posA = std::find(order.begin(), order.end(), SignalId{2});
  auto posB = std::find(order.begin(), order.end(), SignalId{3});
  auto posC = std::find(order.begin(), order.end(), SignalId{4});

  QVERIFY(posA != order.end());
  QVERIFY(posB != order.end());
  QVERIFY(posC != order.end());

  QVERIFY(posA < posC);
  QVERIFY(posB < posC);
}
