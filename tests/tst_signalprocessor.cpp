#include <qtestcase.h>
#include "tst_signalprocessor.h"
#include "buffermanager.h"
#include "calculationplan.h"
#include "calculationcompiler.h"
#include "calculationprocessor.h"
#include "failingformula.h"
#include "formulaadd.h"
#include "formulacopy.h"
#include "formulasqrt.h"
#include "formulatest.h"
#include "parser/formulaparser.h"
#include "parser/identifierresolver.h"
#include "signalprocessor.h"
#include "systemconfiguration.h"
#include "testsrv.h"

tst_signalprocessor::tst_signalprocessor() { }
tst_signalprocessor::~tst_signalprocessor() = default;

static std::optional<std::size_t> findStepIndex(
  std::span<const CalculationStep> steps,
  SignalId id)
{
  auto it = std::find_if(
    steps.begin(),
    steps.end(),
    [id](const CalculationStep& step)
    {
      return step.signal == id;
    });

  if (it == steps.end())
    return std::nullopt;

  return static_cast<std::size_t>(
    std::distance(steps.begin(), it));
}

static qds::CalculationStep findStepOutput(const std::span<const CalculationStep> &steps, SignalId id)
{
  auto index = findStepIndex(steps, id);

  if (!index.has_value())
    return {};

  return steps[index.value()];
}


void tst_signalprocessor::test_calculationPlan_base()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaAstRepository formulas;

  FormulaParser parserA("Raw0");
  QVERIFY(formulas.add(FormulaId{0}, std::move(parserA.parse())));

  FormulaParser parserB("Raw1");
  QVERIFY(formulas.add(FormulaId{1}, std::move(parserB.parse())));

  FormulaParser parserC("A + B");
  QVERIFY(formulas.add(FormulaId{2}, std::move(parserC.parse())));

  CalculationPlan plan;

  CalculationCompiler builder(cfg, layout, formulas);

  QVERIFY(builder.build(plan));

  QCOMPARE(plan.size(), 3);

  const auto& steps = plan.steps();

  auto stepA = findStepOutput(steps, SignalId{17});
  auto stepB = findStepOutput(steps, SignalId{4});
  auto stepC = findStepOutput(steps, SignalId{23});

  auto indexA = findStepIndex(steps, SignalId{17});
  auto indexB = findStepIndex(steps, SignalId{4});
  auto indexC = findStepIndex(steps, SignalId{23});


  QVERIFY(indexA < indexC);
  QVERIFY(indexB < indexC);

  QCOMPARE(stepA.formula, FormulaId{0});
  QCOMPARE(stepB.formula, FormulaId{1});
  QCOMPARE(stepC.formula, FormulaId{2});
}

void tst_signalprocessor::test_calculationPlan_failAst()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaAstRepository formulas;

  FormulaParser parserA("Raw0");
  QVERIFY(formulas.add(FormulaId{0}, std::move(parserA.parse())));

  FormulaParser parserB("Raw1");
  QVERIFY(formulas.add(FormulaId{1}, std::move(parserB.parse())));

  FormulaParser parserC("A + B");
  QVERIFY(formulas.add(FormulaId{3}, std::move(parserC.parse())));

  CalculationPlan plan;

  CalculationCompiler builder(cfg, layout, formulas);

  QVERIFY(!builder.build(plan));

  QCOMPARE(plan.size(), 0);
}

void tst_signalprocessor::test_calculationPlan_cycle()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_cycle();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaAstRepository formulas;

  FormulaParser parserA("abs(B)");
  QVERIFY(formulas.add(FormulaId{0}, std::move(parserA.parse())));

  FormulaParser parserB("abs(C)");
  QVERIFY(formulas.add(FormulaId{1}, std::move(parserB.parse())));

  FormulaParser parserC("abs(A)");
  QVERIFY(formulas.add(FormulaId{2}, std::move(parserC.parse())));

  CalculationPlan plan;

  CalculationCompiler builder(cfg, layout, formulas);

  QVERIFY(!builder.build(plan));

  QCOMPARE(plan.size(), 0);
}

void tst_signalprocessor::test_calculationPlan_rebuildInvalid()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaAstRepository formulas;

  FormulaParser parserA("Raw0");
  QVERIFY(formulas.add(FormulaId{0}, std::move(parserA.parse())));

  FormulaParser parserB("Raw1");
  QVERIFY(formulas.add(FormulaId{1}, std::move(parserB.parse())));

  FormulaParser parserC("A + B");
  QVERIFY(formulas.add(FormulaId{2}, std::move(parserC.parse())));

  CalculationPlan plan;

  CalculationCompiler builder(cfg, layout, formulas);

  QVERIFY(builder.build(plan));

  QCOMPARE(plan.size(), 3);

  formulas.clear();

  QVERIFY(!builder.build(plan));

  QCOMPARE(plan.size(), 0);
}

void tst_signalprocessor::test_signalProcessor_calculate()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_calculate();

  SignalMemoryLayout layout;
  layout.build(cfg);

  RawMemory raw;
  CalculatedMemory calculated;

  raw.initialize(layout.rawSignalCount());
  calculated.initialize(layout.calculatedSignalCount());

  raw.setValue(0, 10.0);
  raw.setValue(1, 20.0);

  FormulaAstRepository formulas;

  FormulaParser parserA("Raw0 + 5");
  QVERIFY(formulas.add(
    FormulaId{0},
    std::move(parserA.parse())));

  FormulaParser parserB("Raw1 * 2");
  QVERIFY(formulas.add(
    FormulaId{1},
    std::move(parserB.parse())));

  FormulaParser parserC("A + B");
  QVERIFY(formulas.add(
    FormulaId{2},
    std::move(parserC.parse())));

  CalculationPlan plan;

  CalculationCompiler builder(cfg, layout, formulas);

  QVERIFY(builder.build(plan));

  QCOMPARE(plan.size(), 3);

  SignalProcessor processor(
    layout,
    formulas,
    plan);

  QVERIFY(processor.process(
    raw,
    calculated));

  QCOMPARE(calculated.valueRef(0), 15.0);
  QCOMPARE(calculated.valueRef(1), 40.0);
  QCOMPARE(calculated.valueRef(2), 55.0);
}

/*
void tst_signalprocessor::test_calculation_plan()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig03();
  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;

  FormulaAstRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));
  QVERIFY(repo.add({5}, std::make_unique<FormulaAdd>()));

  CalculationCompiler builder(cfg, layout, repo);

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

  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));

  CalculationCompiler builder(cfg, layout, repo);

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

  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));
  QVERIFY(repo.add({2}, std::make_unique<FormulaCopy>()));

  CalculationCompiler builder(cfg, layout, repo);

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

  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));

  CalculationCompiler builder(cfg, layout, repo);

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
  SystemConfiguration cfg = createTestConfig_cycle();
  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;

  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));

  CalculationCompiler builder(cfg, layout, repo);

  QVERIFY(!builder.build(plan));
  QCOMPARE(plan.size(), 0);
}

void tst_signalprocessor::test_calculation_selfReference()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_selfReference();
  SignalMemoryLayout layout;
  layout.build(cfg);

  CalculationPlan plan;

  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));

  CalculationCompiler builder(cfg, layout, repo);

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

  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));
  QVERIFY(repo.add({10}, std::make_unique<FormulaAdd>()));
  QVERIFY(repo.add({17}, std::make_unique<FormulaSqrt>()));

  CalculationCompiler builder(cfg, layout, repo);

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

void tst_signalprocessor::test_calculation_calculationProcessor()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Copy_Add();
  SignalMemoryLayout layout;
  layout.build(cfg);
  BufferManager manager;
  manager.initialize(layout);

  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));
  QVERIFY(repo.add({2}, std::make_unique<FormulaAdd>()));

  CalculationCompiler compiler(cfg, layout, repo);

  CalculationPlan plan;

  QVERIFY(compiler.build(plan));
  QCOMPARE(plan.size(), 3);

  // данные попадают в первый блок raw от IDataSource
  // write raw data
  auto &memory = manager.beginWrite();
  std::array<double, 2> block =
    {
      20.0,
      30.0
    };

  memory.raw().setValues(
    0,
    block);

  CalculationProcessor processor(plan);
  // execute calculation
  QVERIFY(processor.process(memory));

  manager.publish();

  // verify calculated values
  auto second = manager.readFrame();

  QCOMPARE(second.calculated().value(0), 20.0);
  QCOMPARE(second.calculated().value(1), 30.0);
  QCOMPARE(second.calculated().value(2), 50.0);
}

void tst_signalprocessor::test_formulas_FormulaRepository()
{
  using namespace qds;
  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));
  QVERIFY(repo.add({2}, std::make_unique<FormulaAdd>()));

  QCOMPARE(repo.size(), 2);

  QVERIFY(!repo.add({0}, std::make_unique<FormulaAdd>()));
  QVERIFY(repo.add({4}, std::make_unique<FormulaCopy>()));

  QCOMPARE(repo.size(), 3);

  auto* f0 = repo.find({2});
  QVERIFY(f0 != nullptr);

  auto* f3 = repo.find({3});
  QVERIFY(f3 == nullptr);

  repo.clear();
  QCOMPARE(repo.size(), 0);
}

void tst_signalprocessor::test_formulas_CalculationPlan()
{
  CalculationPlan plan;
  QCOMPARE(plan.empty(), true);

  plan.clear();

  QCOMPARE(plan.size(), 0);

  auto steps = plan.steps();

  QCOMPARE(steps.size(), 0);
}

void tst_signalprocessor::test_calculationCompiler_failFormula()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Copy_Add();
  SignalMemoryLayout layout;
  layout.build(cfg);
  BufferManager manager;
  manager.initialize(layout);

  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));
  QVERIFY(repo.add({1}, std::make_unique<FormulaAdd>()));
  QVERIFY(repo.add({3}, std::make_unique<FormulaSqrt>()));

  CalculationCompiler compiler(cfg, layout, repo);

  CalculationPlan plan;

  QVERIFY(!compiler.build(plan)); // formula 2 отсутствует
  QCOMPARE(plan.size(), 0);
}

void tst_signalprocessor::test_calculationCompiler_unknownSignalDependency()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfigUnknownDependency();
  SignalMemoryLayout layout;
  layout.build(cfg);
  BufferManager manager;
  manager.initialize(layout);

  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));

  CalculationCompiler compiler(cfg, layout, repo);

  CalculationPlan plan;

  QVERIFY(!compiler.build(plan)); // Отсутствует 8 сигнал
  QCOMPARE(plan.size(), 0);
}

void tst_signalprocessor::test_calculationProcessor_failingFormula()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Copy_Add();
  SignalMemoryLayout layout;
  layout.build(cfg);
  BufferManager manager;
  manager.initialize(layout);

  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));
  QVERIFY(repo.add({2}, std::make_unique<FailingFormula>()));

  CalculationCompiler compiler(cfg, layout, repo);

  CalculationPlan plan;

  QVERIFY(compiler.build(plan));

  auto &memory = manager.beginWrite();
  // Отсутствует ячейка памяти 77
  CalculationProcessor processor(plan);

  QVERIFY(!processor.process(memory));
}

void tst_signalprocessor::test_calculationProcessor_emptyCalculationPlan()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Copy_Add();
  SignalMemoryLayout layout;
  layout.build(cfg);
  BufferManager manager;
  manager.initialize(layout);

  CalculationPlan plan;

  auto &memory = manager.beginWrite();
  // Отсутствует ячейка памяти 77
  CalculationProcessor processor(plan);

  QVERIFY(processor.process(memory));
}

void tst_signalprocessor::test_calculationProcessor_formulaContext()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig_Copy_Add();
  SignalMemoryLayout layout;
  layout.build(cfg);
  BufferManager manager;
  manager.initialize(layout);

  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));
  QVERIFY(repo.add({2}, std::make_unique<FormulaTest>()));

  CalculationCompiler compiler(cfg, layout, repo);

  CalculationPlan plan;

  QVERIFY(compiler.build(plan));
  QCOMPARE(plan.size(), 3);

  auto &memory = manager.beginWrite();
  memory.raw().setValue(0, 1.1);
  memory.raw().setValue(1, 2.2);
  // Отсутствует ячейка памяти 77
  CalculationProcessor processor(plan);

  QVERIFY(processor.process(memory));

  const auto &steps = plan.steps();

  const auto &context2 = static_cast<const FormulaTest*>(steps[2].formula)->context();

  QCOMPARE(context2.inputs.size(), 2);
  QCOMPARE(*context2.inputs[0], 1.1);
  QCOMPARE(*context2.inputs[1], 2.2);
  QCOMPARE(*context2.output, 123.45);

}

void tst_signalprocessor::test_calculationCompiler_unknownSignalOutput()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SystemConfiguration layoutCfg =
    createTestConfig_Copy_Add_WithoutC();

  SignalMemoryLayout layout;
  layout.build(layoutCfg);

  FormulaRepository repo;

  QVERIFY(repo.add(
    {0},
    std::make_unique<FormulaCopy>()));

  QVERIFY(repo.add(
    {2},
    std::make_unique<FormulaAdd>()));

  CalculationCompiler compiler(
    cfg,
    layout,
    repo);

  CalculationPlan plan;

  QVERIFY(!compiler.build(plan));
  QCOMPARE(plan.size(), 0);
}
*/