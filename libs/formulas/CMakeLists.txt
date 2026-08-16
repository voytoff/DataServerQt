cmake_minimum_required(VERSION 3.20)
project(Formulas LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Formulas SHARED
  iformula.h
    formularepository.h formularepository.cpp
  formulacopy.cpp formulacopy.h
  formulaadd.h formulaadd.cpp
  formulacontext.h
  formulasqrt.h formulasqrt.cpp
  failingformula.h
  formulatest.h formulatest.cpp
  iformulafunction.h
  formulafunctionrepository.h formulafunctionrepository.cpp
  formulafunctionsqrt.h formulafunctionsqrt.cpp
  parser/formulanode.h
  parser/formulatoken.h
  parser/formulalexer.h parser/formulalexer.cpp
  parser/formulaparser.h parser/formulaparser.cpp
  parser/identifierresolver.h parser/identifierresolver.cpp
  parser/formulaevaluator.h parser/formulaevaluator.cpp
  formulafunctionabs.h formulafunctionabs.cpp
  formulafunctionmin.h formulafunctionmin.cpp
  formulafunctionmax.h formulafunctionmax.cpp
  calculationorder.h calculationorder.cpp
  formulaastrepository.h formulaastrepository.cpp
  formulacalculator.h formulacalculator.cpp
)

target_include_directories(Formulas
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)

target_link_libraries(Formulas
    PUBLIC
        Configuration
        SignalStorage
)