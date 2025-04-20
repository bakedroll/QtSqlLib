cmake_minimum_required(VERSION 3.10)

set(CMAKE_SCRIPTS_DIRECTORY "" CACHE STRING "The CMake scripts directory")
if(EXISTS ${CMAKE_SCRIPTS_DIRECTORY}/scripts.cmake)
  include(${CMAKE_SCRIPTS_DIRECTORY}/scripts.cmake)
else()
  message(FATAL_ERROR "Please specify CMAKE_SCRIPTS_DIRECTORY")
  return()
endif()

option(QT_USE_VERSION_5 "Use Qt version 5" ON)

project(QtSqlLib)

add_subdirectory(QtSqlLib)
add_subdirectory(QtSqlLibTest)

make_projects()

if (MSVC)
  qt_copy_plugins(platforms/qwindows sqldrivers/qsqlite)
endif()
