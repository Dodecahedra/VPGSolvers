cmake_minimum_required(VERSION 3.17)
project(VPGSolvers)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ../bin/)

add_executable(VPGSolvers
        main.cpp
        VPGame.h VPGame.cpp
        Algorithms/VPG_PP.h Algorithms/VPG_PP.cpp
        Algorithms/VPG_PM.h Algorithms/VPG_PM.cpp
        Conf.h
        VariabilityParityGames/Algorithms/Datastructures/ConfSetExplicit.h
        VariabilityParityGames/Algorithms/Datastructures/ConfSetExplicit.cpp
        VariabilityParityGames/Algorithms/Datastructures/bintree.cpp VariabilityParityGames/Algorithms/Datastructures/bintree.h
        VariabilityParityGames/Algorithms/Datastructures/UnorderedVertexSet.cpp VariabilityParityGames/Algorithms/Datastructures/UnorderedVertexSet.h
        VariabilityParityGames/Algorithms/Datastructures/VectorBoolOptimized.cpp VariabilityParityGames/Algorithms/Datastructures/VectorBoolOptimized.h
        VariabilityParityGames/Algorithms/zlnkVPG.h VariabilityParityGames/Algorithms/zlnkVPG.cpp Algorithms/VPG_SCC.cpp Algorithms/VPG_SCC.h)

find_library(BDD_LIB bdd lib)

target_compile_definitions(VPGSolvers PUBLIC subsetbdd)
target_link_libraries(VPGSolvers LINK_PUBLIC ${BDD_LIB})
