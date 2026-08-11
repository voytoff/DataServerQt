#include <QCoreApplication>
#include <QTest>

#include "tst_archivewriter.h"
#include "tst_configuration.h"
#include "tst_dataarchive.h"
#include "tst_dataprotocol.h"
#include "tst_common.h"
#include "tst_core.h"
#include "tst_datasource.h"
#include "tst_engine.h"
#include "tst_hardware.h"
#include "tst_packetdispatcher.h"
#include "tst_publisher.h"
#include "tst_scheduler.h"
#include "tst_signalprocessor.h"
#include "tst_signalstorage.h"
#include "tst_udpsender.h"
#include "tst_subscriptions.h"
#include "tst_livescheduler.h"
#include "tst_livestorage.h"
#include "tst_udpserver.h"

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  int rc = 0;

  {
    tst_signalprocessor tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_configuration tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_core tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    test_common tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_livescheduler tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_livestorage tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_subscriptions tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_udpsender tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    test_dataprotocol tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_publisher tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_datasource tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_engine tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_hardware tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_udpserver tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_packetdispatcher tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_dataarchive tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_archivewriter tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_signalstorage tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }
  {
    tst_scheduler tc;
    rc |= QTest::qExec(&tc, argc, argv);
  }

  return rc;
}

/*
1.

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

===

#include "formularepository.h"



namespace qds

{



bool FormulaRepository::add(

  FormulaId id,

  std::unique_ptr<IFormula> formula)

{

  auto exists = find(id);



  if (exists)

    return false;



  auto [it, inserted] =

    m_formulas.emplace(id, std::move(formula));



  return inserted;

}



const IFormula*

FormulaRepository::find(

  FormulaId id) const

{

  auto it = m_formulas.find(id);



  if (it == m_formulas.end())

    return nullptr;



  return it->second.get();

}



void FormulaRepository::clear()

{

  m_formulas.clear();

}



std::size_t FormulaRepository::size() const noexcept

{

  return m_formulas.size();

}



}

===

void tst_signalprocessor::test_formulas_CalculationPlan()

{

  CalculationPlan plan;

  QCOMPARE(plan.empty(), true);



  plan.clear();



  QCOMPARE(plan.size(), 0);



  auto steps = plan.steps();



  QCOMPARE(steps.size(), 0);

}



===

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



  QVERIFY(!compiler.build(plan));

  QCOMPARE(plan.size(), 0);

}

===

void tst_signalprocessor::test_calculationCompiler_unknownDependency()
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
но этот тест вылетит тут

SignalReference SignalMemoryLayout::reference(

  SignalId id) const

{

  auto it =

    m_locations.find(id);

  // проверка ошибки конфигурации

  assert(it != m_locations.end());



  return {

    .area = it->second.area,

    .index = it->second.index

  };

}
===
void tst_signalprocessor::test_calculationCompiler_unknownSignalMemoryLayout()
{
  using namespace qds;
  SystemConfiguration cfg = createTestConfig04();
  SignalMemoryLayout layout;
  layout.build(cfg);
  BufferManager manager;
  manager.initialize(layout);

  FormulaRepository repo;
  QVERIFY(repo.add({0}, std::make_unique<FormulaCopy>()));

  cfg.addSignalDefinition({.id = {77}, .name = "U", .kind = SignalKind::Calculated, .archiveFrequency = 100, .formulaId = {0}, .dependencies = {{0}}});
  CalculationCompiler builder(cfg, layout, repo);

  CalculationPlan plan;

  QVERIFY(builder.build(plan));
  QCOMPARE(plan.size(), 3);

  auto &memory = manager.beginWrite();
  // Отсутствует ячейка памяти 77
  CalculationProcessor processor(plan);
  // execute calculation
  QVERIFY(!processor.process(memory));
}


опять валимся тут
SignalReference SignalMemoryLayout::reference(
  SignalId id) const
{
  auto it =
    m_locations.find(id);
  // проверка ошибки конфигурации
  assert(it != m_locations.end());

  return {
    .area = it->second.area,
    .index = it->second.index
  };
}
===

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
===
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
}*/