cmake_minimum_required(VERSION 3.17)
project(VPGSolvers)

set(CMAKE_CXX_STANDARD 14)

add_executable(VPGSolvers
        main.cpp VPGame.h VPGame.cpp VPG_PP.h VPG_PP.cpp)
find_library(BDD_LIB bdd lib)

target_compile_definitions(VPGSolvers PUBLIC subsetbdd)
target_link_libraries(VPGSolvers LINK_PUBLIC ${BDD_LIB})
