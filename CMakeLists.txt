# CMakeList.txt : CMake project for cXml, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("cXml")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ease.xml ${CMAKE_CURRENT_BINARY_DIR}/ease.xml COPYONLY)

# Add source to this project's executable.
add_executable (cXml "cXml.cpp" "cXml.h" "main.cpp")

# TODO: Add tests and install targets if needed.
