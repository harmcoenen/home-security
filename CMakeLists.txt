cmake_minimum_required(VERSION 2.8)

# Set the project name
project(home-security)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Use package config to locate GStreamer
find_package(PkgConfig)
pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.4
                               gstreamer-sdp-1.0>=1.4
                               gstreamer-video-1.0>=1.4
                               gstreamer-app-1.0>=1.4
                               gstreamer-rtsp-server-1.0=1.13.91)
message("-- GST_LIBRARY_DIRS: ${GST_LIBRARY_DIRS}")
message("-- GST_INCLUDE_DIRS: ${GST_INCLUDE_DIRS}")
message("-- GST_CFLAGS:       ${GST_CFLAGS}")
message("-- GST_LIBRARIES:    ${GST_LIBRARIES}")

# - target_include_directories with ${GST_INCLUDE_DIRS}
# - target_compile_options with ${GST_CFLAGS}
# - target_link_libraries with ${GST_LIBRARIES}

# Find jetson and cuda packages
find_package(jetson-utils REQUIRED)
find_package(jetson-inference REQUIRED)
find_package(CUDA REQUIRED)
message("-- CUDA version: ${CUDA_VERSION}")
set(
	CUDA_NVCC_FLAGS
	${CUDA_NVCC_FLAGS}; 
    -O3 
	-gencode arch=compute_53,code=sm_53
	-gencode arch=compute_62,code=sm_62
)

if(CUDA_VERSION_MAJOR GREATER 9)
	message("-- CUDA ${CUDA_VERSION_MAJOR} detected, enabling SM_72")

	set(
		CUDA_NVCC_FLAGS
		${CUDA_NVCC_FLAGS}; 
		-gencode arch=compute_72,code=sm_72
	)

	# OpenCV used for findHomography() and decomposeHomography()
	# OpenCV version >= 3.0.0 required for decomposeHomography()
	find_package(OpenCV 3.0.0 COMPONENTS core calib3d REQUIRED)
endif()
message("-- CUDA_NVCC_FLAGS: ${CUDA_NVCC_FLAGS}")

# Find cURL
find_package(CURL REQUIRED)
message("-- CURL_INCLUDE_DIR: ${CURL_INCLUDE_DIR}")
message("-- CURL_LIBRARIES: ${CURL_LIBRARIES}")

# Adding glib library
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.23)
#include_directories(${GLIB_INCLUDE_DIRS})
#link_directories(${GLIB_LIBRARY_DIRS})
#message("-- GLIB_INCLUDE_DIRS: ${GLIB_INCLUDE_DIRS}")
#message("-- GLIB_LIBRARY_DIRS: ${GLIB_LIBRARY_DIRS}")
#message("-- GLIB_LIBRARIES   : ${GLIB_LIBRARIES}")

# Adding gobject library
pkg_check_modules(GOBJECT REQUIRED gobject-2.0>=2.23)
#include_directories(${GOBJECT_INCLUDE_DIRS})
#link_directories(${GOBJECT_LIBRARY_DIRS})
#message("-- GOBJECT_INCLUDE_DIRS: ${GOBJECT_INCLUDE_DIRS}")
#message("-- GOBJECT_LIBRARY_DIRS: ${GOBJECT_LIBRARY_DIRS}")
#message("-- GOBJECT_LIBRARIES   : ${GOBJECT_LIBRARIES}")

# Include
#include_directories( ${CUDA_INCLUDE_DIRS} )
include_directories( ${GST_INCLUDE_DIRS} )
include_directories( ${CURL_INCLUDE_DIR} )

file(GLOB home-securitySources *.cxx)
file(GLOB home-securityIncludes *.h)

# Compile the home-security program
cuda_add_executable(home-security ${home-securitySources})
#add_executable(home-security ${home-securitySources})

# Link home-security to the jetson-inference library
target_link_libraries(home-security jetson-inference jetson-utils curl ${GOBJECT_LIBRARIES} ${GST_LIBRARIES})

install(TARGETS home-security DESTINATION bin)

