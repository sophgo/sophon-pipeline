cmake_minimum_required(VERSION 3.2)
project(sophon-pipeline_v0.3.8)

if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH x86)
endif()

option(HAVE_TRACKER "Enable tracker" ON)
option(USE_SOPHON_FFMPEG "Use sophon-ffmpeg" ON)
option(USE_SOPHON_OPENCV "Use sophon-opencv" ON)
option(USE_TPU_KERNEL "Use tpu-kernel" OFF)
option(USE_QTGUI "Whether has GUI support" OFF)
option(CMODEL_MODE "Run in cmodel mode" OFF)
option(USE_DEBUG "use debug to print result" OFF)
option(USE_300_SOC "use 300 soc" OFF)

if ((${TARGET_ARCH} STREQUAL "x86") OR (${TARGET_ARCH} STREQUAL "arm64"))
    set(CMAKE_CXX_STANDARD 11)
elseif (${TARGET_ARCH} STREQUAL "soc")
    if (NOT USE_300_SOC)
        set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
        set(CMAKE_ASM_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
        set(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)
    else()
        set(CMAKE_CXX_STANDARD 11)
    endif()
else() 
    message( FATAL_ERROR "${TARGET_ARCH} is not supported yet." )
endif()

add_compile_options(-Wno-deprecated-declarations -fPIC)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
# Global Variables
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(TOP ${CMAKE_SOURCE_DIR})
set(COMMON_TOP ${TOP}/modules)


if (USE_QTGUI)
    if (${TARGET_ARCH} STREQUAL "soc")
        set(QTDIR /opt/qt5.14-18.04-soc)
        set(Qt5Widgets_DIR /opt/qt5.14-18.04-soc/lib/cmake/Qt5Widgets)
    else()
        set(QTDIR /opt/Qt5.12.9/5.12.9/gcc_64)
        set(Qt5Widgets_DIR /opt/Qt5.12.9/5.12.9/gcc_64/lib/cmake/Qt5Widgets)
    endif()
endif()

# User need to set this before compiling.
if (${TARGET_ARCH} STREQUAL "x86")
    if (DEFINED ENV{REL_TOP})
        set(LIBSOPHON_TOP $ENV{REL_TOP})
    else()
        set(LIBSOPHON_TOP /opt/sophon)
    endif()
elseif (${TARGET_ARCH} STREQUAL "soc")
    if (NOT USE_300_SOC)
        set(LIBSOPHON_TOP ${SDK_PATH})
    else()
        set(LIBSOPHON_TOP $ENV{REL_TOP})
    endif()
elseif (${TARGET_ARCH} STREQUAL "arm64")
    set(LIBSOPHON_TOP /opt/sophon)  
else() 
    message( FATAL_ERROR "${TARGET_ARCH} is not supported yet." )
endif()


if ((${TARGET_ARCH} STREQUAL "x86") OR (${TARGET_ARCH} STREQUAL "arm64"))
    find_package(libsophon)
    if(libsophon_FOUND)
        message(STATUS "libsophon include dirs:" ${LIBSOPHON_INCLUDE_DIRS})
        message(STATUS "libsophon lib dirs:" ${LIBSOPHON_LIB_DIRS})

        set(libsophon_inc_dirs ${LIBSOPHON_INCLUDE_DIRS}
            CACHE INTERNAL "")

        # ffmpeg
        set(FFMPEG_DIR  /opt/sophon/sophon-ffmpeg-latest/lib/cmake)
        find_package(FFMPEG REQUIRED NO_DEFAULT_PATH)
        # opencv
        set(OpenCV_DIR /opt/sophon/sophon-opencv-latest/lib/cmake/opencv4)
        find_package(OpenCV REQUIRED NO_DEFAULT_PATH)

        set(sophon_opencv_inc_dirs  ${OpenCV_INCLUDE_DIRS})
        set(sophon_ffmpeg_inc_dirs  ${FFMPEG_INCLUDE_DIRS})

    else()
        set(libsophon_inc_dirs ${LIBSOPHON_TOP}/include
            ${LIBSOPHON_TOP}/include/bmruntime
            ${LIBSOPHON_TOP}/include/bmlib
            ${LIBSOPHON_TOP}/include/third_party/boost/include
            CACHE INTERNAL "")

        set(sophon_opencv_inc_dirs  ${LIBSOPHON_TOP}/include/opencv/opencv4)
        set(sophon_ffmpeg_inc_dirs  ${LIBSOPHON_TOP}/include/ffmpeg ${LIBSOPHON_TOP}/include/bmlib)
    endif(libsophon_FOUND)
elseif (${TARGET_ARCH} STREQUAL "soc")
    if (NOT USE_300_SOC)
        set(libsophon_inc_dirs ${LIBSOPHON_TOP}/include
        CACHE INTERNAL "")
        set(sophon_opencv_inc_dirs  ${LIBSOPHON_TOP}/include/opencv4)
        set(sophon_ffmpeg_inc_dirs  ${LIBSOPHON_TOP}/include)
    else()
        set(libsophon_inc_dirs ${LIBSOPHON_TOP}/include
            ${LIBSOPHON_TOP}/include/bmruntime
            ${LIBSOPHON_TOP}/include/bmlib
            ${LIBSOPHON_TOP}/include/third_party/boost/include
            CACHE INTERNAL "")
        set(sophon_opencv_inc_dirs  ${LIBSOPHON_TOP}/include/opencv/opencv4)
        set(sophon_ffmpeg_inc_dirs  ${LIBSOPHON_TOP}/include/ffmpeg ${LIBSOPHON_TOP}/include/bmlib)
    endif()
else() 
    message( FATAL_ERROR "${TARGET_ARCH} is not supported yet." )
endif()

set(sophon_ffmpeg_link_libs avfilter avformat avcodec avutil avdevice swresample swscale)
set(sophon_opencv_link_libs opencv_core opencv_imgproc opencv_imgcodecs opencv_videoio opencv_dnn)
if (CMODEL_MODE)
  set(libsophon_link_libs bmrt bmcv bmlib gflags glog pthread
        CACHE INTERNAL "")
else()
  set(libsophon_link_libs bmrt bmcv bmvideo bmjpuapi bmjpulite bmlib gflags glog pthread CACHE INTERNAL "")
endif()

if ((${TARGET_ARCH} STREQUAL "x86") OR (${TARGET_ARCH} STREQUAL "arm64"))
    if (libsophon_FOUND)
        set(libsophon_link_dirs ${LIBSOPHON_LIB_DIRS})

        set(sophon_ffmpeg_link_dirs ${FFMPEG_LIB_DIRS})
        set(sophon_ffmpeg_link_libs ${FFMPEG_LIBS})

        set(sophon_opencv_link_dirs ${OpenCV_LIBS})
    else()
        set(sophon_ffmpeg_link_dirs ${LIBSOPHON_TOP}/lib/ffmpeg/pcie ${LIBSOPHON_TOP}/lib/decode/pcie
            ${LIBSOPHON_TOP}/lib/bmnn/pcie
            ${LIBSOPHON_TOP}/lib/thirdparty/x86)
        set(sophon_opencv_link_dirs ${LIBSOPHON_TOP}/lib/opencv/pcie)
        set(libsophon_link_dirs ${LIBSOPHON_TOP}/lib/bmnn/pcie)
    endif(libsophon_FOUND)

elseif (${TARGET_ARCH} STREQUAL "soc")
    if (NOT USE_300_SOC)
        set(sophon_ffmpeg_link_dirs ${LIBSOPHON_TOP}/lib)
        set(sophon_opencv_link_dirs ${LIBSOPHON_TOP}/lib)
        set(libsophon_link_dirs ${LIBSOPHON_TOP}/lib)
    else()
        set(sophon_ffmpeg_link_dirs ${LIBSOPHON_TOP}/lib/ffmpeg/soc ${LIBSOPHON_TOP}/lib/decode/soc
            ${LIBSOPHON_TOP}/lib/bmnn/soc
            ${LIBSOPHON_TOP}/lib/thirdparty/soc)
        set(sophon_opencv_link_dirs ${LIBSOPHON_TOP}/lib/opencv/soc)
        set(libsophon_link_dirs ${LIBSOPHON_TOP}/lib/bmnn/soc)
    endif()

else() 
    message( FATAL_ERROR "${TARGET_ARCH} is not supported yet." )
endif ()

if (USE_SOPHON_FFMPEG)
    set(ffmpeg_inc_dirs ${sophon_ffmpeg_inc_dirs} CACHE INTERNAL "")
    set(ffmpeg_link_dirs ${sophon_ffmpeg_link_dirs} CACHE INTERNAL "")
    set(ffmpeg_link_libs ${sophon_ffmpeg_link_libs} CACHE INTERNAL "")
else()
    set(ffmpeg_inc_dirs /usr/local/include CACHE INTERNAL "")
    set(ffmpeg_link_dirs /usr/local/lib CACHE INTERNAL "")
    set(ffmpeg_link_libs avcodec avutil avformat swscale avdevice CACHE INTERNAL "")
endif()

if (USE_SOPHON_OPENCV)
    set(opencv_inc_dirs ${sophon_opencv_inc_dirs})
    set(opencv_link_dirs ${sophon_opencv_link_dirs})
    set(opencv_link_libs ${sophon_opencv_link_libs})
else()
    find_package(OpenCV REQUIRED)
    set(opencv_inc_dirs ${OpenCV_INCLUDE_DIRS})
    set(opencv_link_dirs ${OpenCV_LIBRARY_DIRS})
    set(opencv_link_libs ${OpenCV_LIBS})
endif()

message(STATUS "ffmpeg_inc = ${ffmpeg_inc_dirs}")
message(STATUS "ffmpeg_link_dirs = ${ffmpeg_link_dirs}")
message(STATUS "opencv_inc = ${opencv_inc_dirs}")
message(STATUS "opencv_link_dirs = ${opencv_link_dirs}")

set(common_inc_dirs ${COMMON_TOP} ${COMMON_TOP}/tracker ${COMMON_TOP}/bmutility ${COMMON_TOP}/bmgui ${TOP}/third_party/jsoncpp ${TOP}/third_party/live555)
message(STATUS "common_inc=${common_inc_dirs}")

add_subdirectory(modules)

add_subdirectory(third_party/jsoncpp)
add_subdirectory(third_party/live555)

if (USE_TPU_KERNEL)
    # Check SDK version by new macro defined in bmlib_runtime.h.
    include(CheckCXXSymbolExists) 
    set(CMAKE_REQUIRED_INCLUDES ${libsophon_inc_dirs})
    check_cxx_symbol_exists(FUNC_MAX_NAME_LEN "bmlib_runtime.h" TPUKERNEL) #check a macro, if exists return 1
    if (TPUKERNEL)
        option(USE_TPUKERNEL "Enable tpukernel postprocess" ON)
    else()
        option(USE_TPUKERNEL "Enable tpukernel postprocess" OFF)
    endif()
else()
    option(USE_TPUKERNEL "Enable tpukernel postprocess" OFF)
endif()

add_subdirectory(examples)

