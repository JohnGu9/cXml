# CMakeList.txt : CMake project for cXml, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("cXml")

configure_file(${PROJECT_SOURCE_DIR}/XmlFIleForTest/ease.xml ${CMAKE_CURRENT_BINARY_DIR}/ease.xml COPYONLY)

include_directories(./source)
add_subdirectory(./source)

# Add source to this project's executable.

add_executable (main "main.cpp")
target_link_libraries(main cXml)

# TODO: Add tests and install targets if needed.
