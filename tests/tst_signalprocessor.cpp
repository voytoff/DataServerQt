#include <qtestcase.h>
#include "tst_signalprocessor.h"
#include "calculationplan.h"
#include "calculationcompiler.h"
#include "systemconfiguration.h"
#include "testsrv.h"

tst_signalprocessor::tst_signalprocessor() { }
tst_signalprocessor::~tst_signalprocessor() = default;

static std::size_t findStepIndex(const std::span<const CalculationStep> &steps, const SignalMemoryLayout &layout, SignalId id)
{
  const SignalReference ref = layout.reference(id);

  auto it = std::find_if(
    steps.begin(),
    steps.end(),
    [&](const CalculationStep& s)
    {
      return s.output.area == ref.area &&
             s.output.index == ref.index;
    });

  if (it != steps.end()) {
    return it - steps.begin(); // Вычисление индекса
  }
  return -1; // Не найдено
}

static qds::CalculationStep findStepOutput(const std::span<const CalculationStep> &steps, const SignalMemoryLayout &layout, SignalId id)
{
  auto index = findStepIndex(steps, layout, id);

  if (index == -1)
    return {};

  return steps[index];
}

void tst_signalprocessor::test_calculation_plan()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig03();
  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;
  FormulaRepository repository;

  CalculationCompiler builder(cfg, layout, repository);

  QVERIFY(builder.build(plan));

  QCOMPARE(plan.size(), 3);
}

void tst_signalprocessor::test_calculation_line()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig04();
  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;
  FormulaRepository repository;

  CalculationCompiler builder(cfg, layout, repository);

  QVERIFY(builder.build(plan));

  QCOMPARE(plan.size(), 3);

  const auto& steps = plan.steps();

  auto posA = findStepOutput(steps, layout, {2});
  auto posB = findStepOutput(steps, layout, {5});
  auto posC = findStepOutput(steps, layout, {10});

  QCOMPARE(posA.inputs.size(), 1);
  QCOMPARE(posA.inputs[0].index, 0);
  QCOMPARE(posA.inputs[0].area, SignalMemoryArea::Raw);
  QCOMPARE(posA.output.index, 0);
  QCOMPARE(posA.output.area, SignalMemoryArea::Calculated);

  QCOMPARE(posB.inputs.size(), 1);
  QCOMPARE(posB.inputs[0].index, 0);
  QCOMPARE(posB.inputs[0].area, SignalMemoryArea::Calculated);
  QCOMPARE(posB.output.index, 1);
  QCOMPARE(posB.output.area, SignalMemoryArea::Calculated);

  QCOMPARE(posC.inputs.size(), 1);
  QCOMPARE(posC.inputs[0].index, 1);
  QCOMPARE(posC.inputs[0].area, SignalMemoryArea::Calculated);
  QCOMPARE(posC.output.index, 2);
  QCOMPARE(posC.output.area, SignalMemoryArea::Calculated);

  auto l0 = layout.reference({0});
  QCOMPARE(l0.index, 0);
  QCOMPARE(l0.area, SignalMemoryArea::Raw);

  auto l1 = layout.reference({2});
  QCOMPARE(l1.index, 0);
  QCOMPARE(l1.area, SignalMemoryArea::Calculated);

  auto l2 = layout.reference({5});
  QCOMPARE(l2.index, 1);
  QCOMPARE(l2.area, SignalMemoryArea::Calculated);

  auto l3 = layout.reference({10});
  QCOMPARE(l3.index, 2);
  QCOMPARE(l3.area, SignalMemoryArea::Calculated);
}

void tst_signalprocessor::test_calculation_branching()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig05();
  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;
  FormulaRepository repository;

  CalculationCompiler builder(cfg, layout, repository);

  QVERIFY(builder.build(plan));

  QCOMPARE(plan.size(), 3);

  const auto& steps = plan.steps();

  auto posA = findStepOutput(steps, layout, {7});
  auto posB = findStepOutput(steps, layout, {11});
  auto posC = findStepOutput(steps, layout, {4});

  QCOMPARE(posA.inputs.size(), 1);
  QCOMPARE(posA.inputs[0].index, 0);
  QCOMPARE(posA.inputs[0].area, SignalMemoryArea::Raw);
  QCOMPARE(posA.output.index, 0);
  QCOMPARE(posA.output.area, SignalMemoryArea::Calculated);

  QCOMPARE(posB.inputs.size(), 1);
  QCOMPARE(posB.inputs[0].index, 1);
  QCOMPARE(posB.inputs[0].area, SignalMemoryArea::Raw);
  QCOMPARE(posB.output.index, 2); // SignalDefinition был позже
  QCOMPARE(posB.output.area, SignalMemoryArea::Calculated);

  QCOMPARE(posC.inputs.size(), 1);
  QCOMPARE(posC.inputs[0].index, 0);
  QCOMPARE(posC.inputs[0].area, SignalMemoryArea::Calculated);
  QCOMPARE(posC.output.index, 1); // SignalDefinition был раньше
  QCOMPARE(posC.output.area, SignalMemoryArea::Calculated);

  auto indexA = findStepIndex(steps, layout, {7});
  auto indexB = findStepIndex(steps, layout, {11});
  auto indexC = findStepIndex(steps, layout, {4});

  QVERIFY(indexA < indexC);
  QVERIFY(indexB < indexC);
}

void tst_signalprocessor::test_calculation_independent()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig06();
  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;
  FormulaRepository repository;

  CalculationCompiler builder(cfg, layout, repository);

  QVERIFY(builder.build(plan));

  QCOMPARE(plan.size(), 3);

  const auto& steps = plan.steps();

  auto posA = findStepOutput(steps, layout, {101});
  auto posB = findStepOutput(steps, layout, {70});
  auto posC = findStepOutput(steps, layout, {4});

  QCOMPARE(posA.inputs.size(), 1);
  QCOMPARE(posA.inputs[0].index, 0);
  QCOMPARE(posA.inputs[0].area, SignalMemoryArea::Raw);
  QCOMPARE(posA.output.index, 0);
  QCOMPARE(posA.output.area, SignalMemoryArea::Calculated);

  QCOMPARE(posB.inputs.size(), 1);
  QCOMPARE(posB.inputs[0].index, 0);
  QCOMPARE(posB.inputs[0].area, SignalMemoryArea::Raw);
  QCOMPARE(posB.output.index, 1);
  QCOMPARE(posB.output.area, SignalMemoryArea::Calculated);

  QCOMPARE(posC.inputs.size(), 1);
  QCOMPARE(posC.inputs[0].index, 0);
  QCOMPARE(posC.inputs[0].area, SignalMemoryArea::Raw);
  QCOMPARE(posC.output.index, 2);
  QCOMPARE(posC.output.area, SignalMemoryArea::Calculated);
}

void tst_signalprocessor::test_calculation_cycle()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig07();
  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;
  FormulaRepository repository;

  CalculationCompiler builder(cfg, layout, repository);

  QVERIFY(!builder.build(plan));
  QCOMPARE(plan.size(), 0);
}

void tst_signalprocessor::test_calculation_selfReference()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig08();
  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;
  FormulaRepository repository;

  CalculationCompiler builder(cfg, layout, repository);

  QVERIFY(!builder.build(plan));
  QCOMPARE(plan.size(), 0);
}

void tst_signalprocessor::test_calculation_bigGraph()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig09();
  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;
  FormulaRepository repository;

  CalculationCompiler builder(cfg, layout, repository);

  QVERIFY(builder.build(plan));

  QCOMPARE(plan.size(), 5);

  const auto& steps = plan.steps();

  auto posA = findStepOutput(steps, layout, {7});
  auto posB = findStepOutput(steps, layout, {4});
  auto posC = findStepOutput(steps, layout, {11});
  auto posD = findStepOutput(steps, layout, {10});
  auto posE = findStepOutput(steps, layout, {3});

  QCOMPARE(posA.inputs.size(), 1);
  QCOMPARE(posA.inputs[0].index, 0);
  QCOMPARE(posA.inputs[0].area, SignalMemoryArea::Raw);
  QCOMPARE(posA.output.index, 0);
  QCOMPARE(posA.output.area, SignalMemoryArea::Calculated);

  QCOMPARE(posB.inputs.size(), 1);
  QCOMPARE(posB.inputs[0].index, 1);
  QCOMPARE(posB.inputs[0].area, SignalMemoryArea::Raw);
  QCOMPARE(posB.output.index, 1);
  QCOMPARE(posB.output.area, SignalMemoryArea::Calculated);

  QCOMPARE(posC.inputs.size(), 2);
  QVERIFY(posC.inputs[0].index == 0 || posC.inputs[0].index == 1);
  QCOMPARE(posC.inputs[0].area, SignalMemoryArea::Calculated);
  QVERIFY(posC.inputs[1].index == 0 || posC.inputs[1].index == 1);
  QCOMPARE(posC.inputs[1].area, SignalMemoryArea::Calculated);
  QCOMPARE(posC.output.index, 2);
  QCOMPARE(posC.output.area, SignalMemoryArea::Calculated);
  QVERIFY(posC.inputs[0].index != posC.inputs[1].index);

  QCOMPARE(posD.inputs.size(), 1);
  QCOMPARE(posD.inputs[0].index, 2);
  QCOMPARE(posD.inputs[0].area, SignalMemoryArea::Calculated);
  QCOMPARE(posD.output.index, 3);
  QCOMPARE(posD.output.area, SignalMemoryArea::Calculated);

  QCOMPARE(posE.inputs.size(), 2);
  QVERIFY(posE.inputs[0].index == 1 || posE.inputs[0].index == 3);
  QCOMPARE(posE.inputs[0].area, SignalMemoryArea::Calculated);
  QVERIFY(posE.inputs[1].index == 1 || posE.inputs[1].index == 3);
  QCOMPARE(posE.inputs[1].area, SignalMemoryArea::Calculated);
  QCOMPARE(posE.output.index, 4);
  QCOMPARE(posE.output.area, SignalMemoryArea::Calculated);
  QVERIFY(posE.inputs[0].index != posE.inputs[1].index);
}
