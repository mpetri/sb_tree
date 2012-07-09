cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW) 

project(SBTREE CXX C)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -msse4.2 -funroll-loops -O3")

INCLUDE_DIRECTORIES($ENV{HOME}/include)
LINK_DIRECTORIES($ENV{HOME}/lib)

ADD_EXECUTABLE(sb-tree-build sb-tree-build.cpp sb_tree.cpp sb_tmpfile.cpp critbit_tree.cpp)
TARGET_LINK_LIBRARIES(sb-tree-build sdsl divsufsort64)
#SET_TARGET_PROPERTIES(neWT-build-imp PROPERTIES COMPILE_FLAGS "-fopenmp -O3 -msse4.2 -mpopcnt -funroll-loops")

ADD_EXECUTABLE(sb-tree-build-dbg sb_tree.cpp sb-tree-build.cpp sb_tmpfile.cpp critbit_tree.cpp)
TARGET_LINK_LIBRARIES(sb-tree-build-dbg sdsl divsufsort64)
#SET_TARGET_PROPERTIES(neWT-build-imp-dbg PROPERTIES COMPILE_FLAGS "-fopenmp -O0 -g")

ADD_EXECUTABLE(critbit_test critbit_test.cpp critbit_tree.cpp)
TARGET_LINK_LIBRARIES(critbit_test sdsl gtest pthread)
SET_TARGET_PROPERTIES(critbit_test PROPERTIES COMPILE_FLAGS "-fopenmp -O0 -g")

ENABLE_TESTING()
ADD_TEST(CritBitTest ${CURRENT_BINARY_DIR}/critbit_test)