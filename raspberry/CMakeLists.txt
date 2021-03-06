project (robot_raspi)
cmake_minimum_required (VERSION 2.6)

# Set BOOST_ROOT and OpenCV_DIR environment variables
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# set(Boost_DEBUG ON)
# set(OpenSSL_DEBUG ON)
find_package(Boost 1.55.0 REQUIRED COMPONENTS system program_options)
find_package(OpenCV REQUIRED)
find_package(LibMicroHTTPD REQUIRED)
# find_package(OpenSSL REQUIRED)

# libicp uses openmp, it's not supported on the Mac, but is on the raspi
option(USE_OPENMP "Enable OpenMP?" ON) # set to OFF to disable
if(USE_OPENMP)
 FIND_PACKAGE(OpenMP)
 if(OPENMP_FOUND OR OpenMP_FOUND)
  message(STATUS "OpenMP flags = ${OpenMP_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
 else(OPENMP_FOUND OR OpenMP_FOUND)
  message(STATUS "OpenMP not supported")
 endif(OPENMP_FOUND OR OpenMP_FOUND)
endif(USE_OPENMP)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${MHD_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories("../arduino/src")
include_directories("libicp/src")

add_executable(robot
    occupancy_grid.h
    occupancy_grid.inl
	occupancy_grid.cpp
	deadreckoning.h
	deadreckoning.cpp
    scanline.h
	scanline.cpp
    scanmatching.h
	scanmatching.cpp
	robot_strategy.cpp
	path_finding.cpp
	main.cpp
    error_handling.h
	error_handling.cpp
    robot_configuration.h
	robot_configuration.cpp
	robot_connection.cpp
    fast_particle_slam.h
    fast_particle_slam.cpp
    particle_slam.h
	particle_slam.cpp
    parse_log_file.cpp
	libicp/src/icp.h
	libicp/src/icp.cpp
	libicp/src/icpPointToPlane.h
	libicp/src/icpPointToPlane.cpp
	libicp/src/icpPointToPoint.h
	libicp/src/icpPointToPoint.cpp
	libicp/src/kdtree.h
	libicp/src/kdtree.cpp
	libicp/src/matrix.h
	libicp/src/matrix.cpp)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/opt/openssl/lib")

target_compile_options(robot PRIVATE --std=c++1y -DBOOST_RANGE_ENABLE_CONCEPT_ASSERT=0 -Wall -Wno-deprecated-declarations -Wno-unused-local-typedef)

target_link_libraries( robot 
	${OpenCV_LIBS} 
	${Boost_LIBRARIES}
	${MHD_LIBRARIES}
	pthread
)
