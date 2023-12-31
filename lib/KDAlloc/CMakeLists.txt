#===------------------------------------------------------------------------===#
#
#                     The KLEE Symbolic Virtual Machine
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===------------------------------------------------------------------------===#
add_library(KDAlloc SHARED
  lib.cpp
  log.cpp
)


target_include_directories(KDAlloc PRIVATE ${KLEE_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
target_compile_options(KDAlloc PRIVATE ${KLEE_COMPONENT_CXX_FLAGS} -fvisibility=hidden)
target_compile_definitions(KDAlloc PRIVATE ${KLEE_COMPONENT_CXX_DEFINES})

install(TARGETS KDAlloc DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}")


add_library(KDAllocAsan STATIC
  asan.cpp
)

target_include_directories(KDAllocAsan PRIVATE ${KLEE_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
target_compile_options(KDAllocAsan PRIVATE ${KLEE_COMPONENT_CXX_FLAGS})
target_compile_definitions(KDAllocAsan PUBLIC ${KLEE_COMPONENT_CXX_DEFINES})

install(TARGETS KDAllocAsan DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}")
