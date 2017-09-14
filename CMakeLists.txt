cmake_minimum_required(VERSION 3.7)

project(squeeze CXX)

if(CMAKE_BUILD_TYPE STREQUAL Release OR NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    set(DEBUG FALSE)
else()
    set(CMAKE_BUILD_TYPE Debug)
    set(DEBUG TRUE)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

set(SQUEEZE_DIR ${CMAKE_SOURCE_DIR})
set(SQUIRREL_DIR ${CMAKE_SOURCE_DIR}/squirrel)
set(CPPUTEST_DIR ${CMAKE_SOURCE_DIR}/cpputest)

set(SQUEEZE_INCLUDE_DIR ${SQUEEZE_DIR}/src)
set(SQUIRREL_INCLUDE_DIR ${SQUIRREL_DIR}/include)
set(CPPUTEST_INCLUDE_DIR ${CPPUTEST_DIR}/include)

if(DEBUG)
    set(SQUIRREL_LIB_DIR ${SQUIRREL_DIR}/lib/debug)
    set(CPPUTEST_LIB_DIR ${CPPUTEST_DIR}/lib/debug)
else()
    set(SQUIRREL_LIB_DIR ${SQUIRREL_DIR}/lib/release)
    set(CPPUTEST_LIB_DIR ${CPPUTEST_DIR}/lib/release)
endif()

add_subdirectory(${SQUEEZE_DIR}/src squeeze)
add_subdirectory(${CMAKE_SOURCE_DIR}/tests)