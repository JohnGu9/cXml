# CMakeList.txt : CMake project for cXml, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("cXml")

# copy test.xml to runtime directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test.xml ${CMAKE_CURRENT_BINARY_DIR}/test.xml COPYONLY)

include_directories(./source)
add_subdirectory(./source)

# Add source to this project's executable.

add_executable (cXmlDemo "main.cpp")
target_link_libraries(cXmlDemo cXml)

# TODO: Add tests and install targets if needed.
