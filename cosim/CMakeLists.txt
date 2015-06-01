# Copyright (c) 2015 Matthias Noack <ma.noack.pr@gmail.com>
#
# See accompanying file LICENSE and README for further information.

cmake_minimum_required (VERSION 2.6)
project (CoSimPro) # must be different from executable

# http://www.cmake.org/Wiki/Eclipse_CDT4_Generator

set(COVIS_DIR ${PROJECT_SOURCE_DIR}/../covis)
set(THIRDPARTY_DIR ${PROJECT_SOURCE_DIR}/../thirdparty)
set(OpenCL_KERNEL_DIR ${PROJECT_SOURCE_DIR}/cl)

# compiler options
list(APPEND CMAKE_CXX_FLAGS "-std=c++11 -Wall ${CMAKE_CXX_FLAGS}")

# executable
add_executable(cosim src/cosim.cpp src/BodyParticleSystem src/ComputeConfig.cpp ${COVIS_DIR}/src/ConfigParser.cpp)
add_executable(oclinfo src/oclinfo.cpp)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${COVIS_DIR}/include)
include_directories(${OpenCL_KERNEL_DIR})

# preprocess OpenCL code into a header containing an character array and length
# output header depends on input cl file
add_custom_command(OUTPUT ${OpenCL_KERNEL_DIR}/integrate_eom_kernel.h 
	COMMAND xxd -i integrate_eom_kernel.cl integrate_eom_kernel.h 
	WORKING_DIRECTORY ${OpenCL_KERNEL_DIR}
	COMMENT "Generating OpenCL kernel header"
	DEPENDS ${OpenCL_KERNEL_DIR}/integrate_eom_kernel.cl)

# create dependencie between generated OpenCL header and cpp file using it
SET_SOURCE_FILES_PROPERTIES(src/cosim.cpp PROPERTIES OBJECT_DEPENDS ${OpenCL_KERNEL_DIR}/integrate_eom_kernel.h)

# OpenCL
set(CMAKE_MODULE_PATH ${THIRDPARTY_DIR}/cmake/Modules/)
find_package(OpenCL REQUIRED)

# thirdparty
set(THIRD_PARTY_INCLUDE ${THIRDPARTY_DIR}/include)
set(THIRD_PARTY_LIBS ${THIRDPARTY_DIR}/lib)

include_directories(${OpenCL_INCLUDE_DIRS} ${THIRD_PARTY_INCLUDE})
target_link_libraries(cosim ${OpenCL_LIBRARIES} ${THIRD_PARTY_LIBS}/libtinyobjloader.a)
target_link_libraries(oclinfo ${OpenCL_LIBRARIES})

