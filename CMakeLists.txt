cmake_minimum_required(VERSION 3.17)
project(VPGSolvers)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ../bin/)

add_executable(VPGSolvers_PP
        main.cpp
        VPGame.h VPGame.cpp
        VPG_PP.h VPG_PP.cpp
        Conf.h
        VariabilityParityGames/Algorithms/Datastructures/ConfSetExplicit.h
        VariabilityParityGames/Algorithms/Datastructures/ConfSetExplicit.cpp
        VariabilityParityGames/Algorithms/Datastructures/bintree.cpp VariabilityParityGames/Algorithms/Datastructures/bintree.h VariabilityParityGames/Algorithms/Datastructures/UnorderedVertexSet.cpp VariabilityParityGames/Algorithms/Datastructures/UnorderedVertexSet.h VariabilityParityGames/Algorithms/Datastructures/VectorBoolOptimized.cpp VariabilityParityGames/Algorithms/Datastructures/VectorBoolOptimized.h)
add_executable(VPGSolvers_PM
        main.cpp
        VPGame.h VPGame.cpp
        VPG_PP.h VPG_PP.cpp
        VPG_PM.h VPG_PM.cpp
        Conf.h
        VariabilityParityGames/Algorithms/Datastructures/ConfSetExplicit.h
        VariabilityParityGames/Algorithms/Datastructures/ConfSetExplicit.cpp
        VariabilityParityGames/Algorithms/Datastructures/bintree.cpp VariabilityParityGames/Algorithms/Datastructures/bintree.h VariabilityParityGames/Algorithms/Datastructures/UnorderedVertexSet.cpp VariabilityParityGames/Algorithms/Datastructures/UnorderedVertexSet.h VariabilityParityGames/Algorithms/Datastructures/VectorBoolOptimized.cpp VariabilityParityGames/Algorithms/Datastructures/VectorBoolOptimized.h)

find_library(BDD_LIB bdd lib)

target_compile_definitions(VPGSolvers_PP PUBLIC subsetbdd)
target_link_libraries(VPGSolvers_PP LINK_PUBLIC ${BDD_LIB})

target_compile_definitions(VPGSolvers_PM PUBLIC subsetbdd)
target_link_libraries(VPGSolvers_PM LINK_PUBLIC ${BDD_LIB})
