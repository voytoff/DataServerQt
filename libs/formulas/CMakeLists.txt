cmake_minimum_required(VERSION 3.23)
project(formulas LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(formulas SHARED
  iformula.h
  formulacontext.h
  failingformula.h
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
  formulabuilder.h formulabuilder.cpp
)

target_include_directories(formulas
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../datatypes
)

target_link_libraries(formulas
  PUBLIC
    configuration
    signalstorage
)