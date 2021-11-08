cmake_minimum_required(VERSION 3.17)
project(VPGSolvers)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ../bin/)

set(BUILD_SHARED_LIBS ON)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        703bd9caab50b139428cea1aaff9974ebee5742e # release-1.10.0
)
FetchContent_MakeAvailable(googletest)

add_executable(VPGSolvers
        main.cpp
        VPGame.h VariabilityParityGames/VPGame.cpp
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

enable_testing()
add_executable(
        VPG_tests
        VariabilityParityGames/VPGame.h VariabilityParityGames/VPGame.cpp
        VariabilityParityGames/Algorithms/Datastructures/VectorBoolOptimized.cpp VariabilityParityGames/Algorithms/Datastructures/VectorBoolOptimized.h
        VariabilityParityGames/Algorithms/zlnkVPG.h VariabilityParityGames/Algorithms/zlnkVPG.cpp
        Conf.h
        Algorithms/VPG_PP.cpp Algorithms/VPG_PP.h
        Algorithms/VPG_PM.cpp Algorithms/VPG_PM.h
        Algorithms/VPG_SCC.cpp Algorithms/VPG_SCC.h
        test/src/VPG_test.cpp
        test/src/VPG_PP_test.cpp
        test/src/VPG_PM_test.cpp
        test/src/VPG_SCC_test.cpp test/src/VPG_test.cpp)
target_compile_definitions(VPG_tests PUBLIC subsetbdd)
target_link_libraries(VPG_tests LINK_PUBLIC gtest_main ${BDD_LIB})
include(GoogleTest)
gtest_discover_tests(VPG_tests)