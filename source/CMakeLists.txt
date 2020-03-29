# CMakeList.txt : CMake project for cXml, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

aux_source_directory(. CMXL_DIR_SRCS)

option(DLL_LINK "" OFF)

if(DLL_LINK)
	add_definitions("-_XML_DLL")
	add_library(cXml SHARED ${CMXL_DIR_SRCS})
else(DLL_LINK)
	add_library(cXml STATIC ${CMXL_DIR_SRCS})
endif(DLL_LINK)

# TODO: Add tests and install targets if needed.
