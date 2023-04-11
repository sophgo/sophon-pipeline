cmake_minimum_required(VERSION 3.2)
project(bmtracker)

set(TRACKER_TOP ${COMMON_TOP}/tracker)
if (${TARGET_ARCH} STREQUAL "x86")
    set(eigen3_inc_dirs /usr/include)
elseif (${TARGET_ARCH} STREQUAL "soc")
    if (NOT USE_300_SOC)
        set(eigen3_inc_dirs ${libsophon_inc_dirs})
    else()
        set(eigen3_inc_dirs /usr/include)
    endif()
elseif (${TARGET_ARCH} STREQUAL "arm64")
    set(eigen3_inc_dirs /usr/local/include)
else() 
    message( FATAL_ERROR "${TARGET_ARCH} is not supported yet." )
endif()
include_directories(${common_inc_dirs} ${libsophon_inc_dirs} ${sophon_ffmpeg_inc_dirs} ${sophon_opencv_inc_dirs} ${eigen3_inc_dirs}/eigen3)

add_library(bmtracker bm_tracker.cpp
        ${TRACKER_TOP}/KalmanFilter/kalmanfilter.cpp
        ${TRACKER_TOP}/KalmanFilter/linear_assignment.cpp
        ${TRACKER_TOP}/KalmanFilter/nn_matching.cpp
        ${TRACKER_TOP}/KalmanFilter/track.cpp
        ${TRACKER_TOP}/KalmanFilter/tracker.cpp
        ${TRACKER_TOP}/MunkresAssignment/hungarianoper.cpp
        ${TRACKER_TOP}/MunkresAssignment/munkres/munkres.cpp
        ${TRACKER_TOP}/DeepAppearanceDescriptor/model.cpp
        )
