cmake_minimum_required(VERSION 3.5)

set(PACKAGE "SPHEREGENERATOR")
set(CMAKE_BUILD_TYPE Release)
project(${PACKAGE})

IF (WIN32)
  SET(CMAKE_CXX_COMPILER "x86_64-w64-mingw32-g++")
ELSE()
  find_package(PkgConfig)
ENDIF()

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

include_directories(include)

set( EXECUTABLES SphereGenerator )

set (CMAKE_CXX_STANDARD 11)

foreach( EXECUTABLE ${EXECUTABLES} )
    add_executable( Run${EXECUTABLE} lib/Run${EXECUTABLE}.c++ lib/${EXECUTABLE}.c++ )
    target_link_libraries(Run${EXECUTABLE} ${GLM_LIBRARY_DIRS})
endforeach( EXECUTABLE ${EXECUTABLES} )
