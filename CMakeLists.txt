cmake_minimum_required(VERSION 3.2)
project(sophon-pipeline_v0.2.0)
# set(CMAKE_CXX_FLAGS "-O3")

if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH x86)
endif()

if (${TARGET_ARCH} STREQUAL "x86")
    set(CMAKE_CXX_STANDARD 11)
elseif (${TARGET_ARCH} STREQUAL "soc")
    set(CMAKE_C_COMPILER   /usr/bin/aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)
    # set(CMAKE_C_COMPILER   /home/lihengfang/work/aarch64-gcc/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc)
    # set(CMAKE_ASM_COMPILER /home/lihengfang/work/aarch64-gcc/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc)
    # set(CMAKE_CXX_COMPILER /home/lihengfang/work/aarch64-gcc/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++)
else() 
    message( FATAL_ERROR "${TARGET_ARCH} is not supported yet." )
endif()


option(HAVE_TRACKER "Enable tracker" OFF)
option(USE_SOPHON_FFMPEG "Use sophon-ffmpeg" ON)
option(USE_SOPHON_OPENCV "Use sophon-opencv" OFF)
option(CMODEL_MODE "Run in cmodel mode" OFF)
option(USE_DEBUG "use debug to print result" OFF)
option(USE_QTGUI "Whether has GUI support" OFF)
option(A2_SDK "use A2 SDK" ON)
option(PLD "use PLD debug info" OFF)
option(WITH_DECODE "with decode process" ON)
option(WITH_HDMI "with hdmi encode" OFF)
option(WITH_DETECTOR "with detector" OFF)
option(WITH_EXTRACTOR "with extractor" OFF)
option(WITH_OUTPUTER "with client outputer" OFF)

if(A2_SDK)
    add_definitions(-DA2_SDK=1)
else()
    add_definitions(-DA2_SDK=0)
endif()
if(PLD)
    add_definitions(-DPLD=1)
else()
    add_definitions(-DPLD=0)
endif()
if(WITH_DECODE)
    add_definitions(-DWITH_DECODE=1)
else()
    add_definitions(-DWITH_DECODE=0)
endif()
if(WITH_HDMI)
    # add_definitions(-DWITH_ENCODE_JPEG=1)   #depend on decode
    add_definitions(-DWITH_HDMI=1)          #depend on encode_jpeg\QT_GUI, a2 hdmi
elseif(WITH_ENCODE)
    if(ENC_JPEG)
        add_definitions(-DWITH_ENCODE_JPEG=1)   #depend on decode
    elseif(ENC_H264)
        add_definitions(-DWITH_ENCODE_H264=1)   #depend on decode
    endif()
    add_definitions(-DWITH_HDMI=0)          #depend on encode_jpeg\QT_GUI, a2 hdmi
else()
    add_definitions(-DWITH_ENCODE_JPEG=0)   #depend on decode
    add_definitions(-DWITH_HDMI=0)          #depend on encode_jpeg\QT_GUI, a2 hdmi
endif()
if(WITH_DETECTOR)
    add_definitions(-DWITH_DETECTOR=1)      #depend on decode
else()
    add_definitions(-DWITH_DETECTOR=0)      #depend on decode
endif()
if(WITH_EXTRACTOR)
    add_definitions(-DWITH_EXTRACTOR=1)
else()
    add_definitions(-DWITH_EXTRACTOR=0)
endif()
if(WITH_OUTPUTER)
    add_definitions(-DWITH_OUTPUTER=1)     #depend on detector, output rtsp flow to pipeline client.
else()
    add_definitions(-DWITH_OUTPUTER=0)
endif()

if(TEST_PREPROCESS)
    add_definitions(-DWITH_RESIZE_TEST=1)   #depend on decode\encode_jpeg.
    add_definitions(-DWITH_CONVERTO_TEST=1) #depend on decode\encode_jpeg\resize_test.
endif()
add_definitions(-DWITH_FAKEDATA=0)      #depend on detector, feed fake data to postprocess.
add_definitions(-DDRAW_DETECTOR=0)      #depend on detector. Func: draw boxes and encode.
add_definitions(-DDRAR_EXTRACTOR=0)     #depend on extractor (Not implement yet)
add_definitions(-DBM_THREAD_SYNC=1)     #if use bm_thread_sync
add_definitions(-DFLOW_CONTROL=1)       #flow control
add_definitions(-DWITH_PROFILE=0)       #tpu/cpu/mem profile
add_definitions(-DSAVE_QT_FRAME=0)
# add_definitions(-DPRINT_QUEUE=1)        #print blocking queue name and pop size
add_definitions(-DWITH_LOAD=1)          #load bmodel
add_definitions(-DDEBUG_SYNC=1)         #std::future async debug
add_definitions(-DPREPROCESS_TIMER=0)
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
	    #set(QTDIR /opt/qt5.14-18.04-soc)
        set(QTDIR ${QT_PATH})
        set(Qt5Widgets_DIR ${QT_PATH}/lib/cmake/Qt5Widgets)
	#set(Qt5Widgets_DIR /opt/qt5.14-18.04-soc/lib/cmake/Qt5Widgets)
    else()
        # message(STATUS "Qt5_INCLUDE_DIRS: ${Qt5_INCLUDE_DIRS}")
        # message(STATUS "Qt5_LIBRARIES: ${Qt5_LIBRARIES}")
        # message(STATUS "Qt5_VERSION: ${Qt5_VERSION}")
        set(QTDIR /opt/qt-5.15/)
        set(Qt5Widgets_DIR /opt/qt-5.15/lib/cmake/Qt5Widgets)
    endif()
endif()

message("=================")
# User need to set this before compiling.
if (${TARGET_ARCH} STREQUAL "x86")
    if (DEFINED ENV{REL_TOP})
        set(LIBSOPHON_TOP $ENV{REL_TOP})
    else()
        set(LIBSOPHON_TOP /opt/sophon)
    endif()
elseif (${TARGET_ARCH} STREQUAL "soc")
    set(LIBSOPHON_TOP ${SDK_PATH})  
else() 
    message( FATAL_ERROR "${TARGET_ARCH} is not supported yet." )
endif()


if (${TARGET_ARCH} STREQUAL "x86")
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
    set(libsophon_inc_dirs ${LIBSOPHON_TOP}/include
    CACHE INTERNAL "")
    if (A2_SDK AND (NOT USE_SOPHON_OPENCV))
        set(sophon_opencv_inc_dirs  ${LIBSOPHON_TOP}/official_opencv/include) #official opencv
    else()
        set(sophon_opencv_inc_dirs  ${LIBSOPHON_TOP}/include/opencv4)
    endif()
    set(sophon_ffmpeg_inc_dirs  ${LIBSOPHON_TOP}/include)
else() 
    message( FATAL_ERROR "${TARGET_ARCH} is not supported yet." )
endif()

message("====================")
message("USING official OPENCV")
message("USE_SOPHON_OPENCV: ${USE_SOPHON_OPENCV} ${sophon_opencv_inc_dirs}, ${OpenCV_LIBS}")

set(sophon_ffmpeg_link_libs avfilter avformat avcodec avutil avdevice)# swresample swscale
set(sophon_opencv_link_libs opencv_core opencv_imgproc opencv_imgcodecs)#opencv_videoio opencv_dnn
if (CMODEL_MODE)
  set(libsophon_link_libs bmrt bmcv bmlib gflags glog pthread
        CACHE INTERNAL "")
else()
  set(libsophon_link_libs bmrt bmcv bmlib pthread CACHE INTERNAL "") # gflags glog bmvideo bmjpuapi bmjpulite 
endif()

if (${TARGET_ARCH} STREQUAL "x86")
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

elseif (${TARGET_ARCH} STREQUAL "arm64")
    # arm-pcie
    set(sophon_ffmpeg_link_dirs ${LIBSOPHON_TOP}/lib/ffmpeg/arm_pcie ${LIBSOPHON_TOP}/lib/decode/arm_pcie
            ${LIBSOPHON_TOP}/lib/bmnn/arm_pcie
            ${LIBSOPHON_TOP}/lib/thirdparty/soc)
    set(sophon_opencv_link_dirs ${LIBSOPHON_TOP}/lib/opencv/arm_pcie)
    set(libsophon_link_dirs ${LIBSOPHON_TOP}/lib/bmnn/arm_pcie)

elseif (${TARGET_ARCH} STREQUAL "soc")
    set(sophon_ffmpeg_link_dirs ${LIBSOPHON_TOP}/lib)
    if (A2_SDK AND (NOT USE_SOPHON_OPENCV))
        set(sophon_opencv_link_dirs ${LIBSOPHON_TOP}/official_opencv/lib) #official opencv
    else()
        set(sophon_opencv_link_dirs ${LIBSOPHON_TOP}/lib)
    endif()
    set(libsophon_link_dirs ${LIBSOPHON_TOP}/lib)
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
    add_definitions(-DUSE_SOPHON_OPENCV=1)
    set(opencv_inc_dirs ${sophon_opencv_inc_dirs})
    set(opencv_link_dirs ${sophon_opencv_link_dirs})
    set(opencv_link_libs ${sophon_opencv_link_libs})
else()
    # find_package(OpenCV REQUIRED)
    # set(opencv_inc_dirs ${OpenCV_INCLUDE_DIRS})
    # set(opencv_link_dirs ${OpenCV_LIBRARY_DIRS})
    # set(opencv_link_libs ${OpenCV_LIBS})
    set(opencv_inc_dirs ${sophon_opencv_inc_dirs})
    set(opencv_link_dirs ${sophon_opencv_link_dirs})
    set(opencv_link_libs ${sophon_opencv_link_libs})
endif()


message(STATUS "ffmpeg_inc = ${ffmpeg_inc_dirs}")
message(STATUS "ffmpeg_link_dirs = ${ffmpeg_link_dirs}")
message(STATUS "opencv_inc = ${opencv_inc_dirs}")
message(STATUS "opencv_link_dirs = ${opencv_link_dirs}")

set(common_inc_dirs ${COMMON_TOP} ${COMMON_TOP}/tracker ${COMMON_TOP}/bmutility ${COMMON_TOP}/bmgui ${TOP}/third_party/jsoncpp)
message(STATUS "common_inc=${common_inc_dirs}")

add_subdirectory(modules)

add_subdirectory(third_party/jsoncpp)
#add_subdirectory(third_party/live555)

add_subdirectory(examples)


