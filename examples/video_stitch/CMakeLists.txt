cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wno-deprecated-declarations)


set(rtsp_srcs 
    rtsp/BTRTSPServer.cpp 
    rtsp/Live555RtspServer.cpp
    rtsp/H264FrameSource.cpp
    rtsp/H264QueMediaSubsession.cpp
)
set(src_files main.cpp worker.cpp yolov5s.cpp stitch.cpp encoder.cpp ${rtsp_srcs})

if(USE_DEBUG)
    add_definitions(-DUSE_DEBUG=1)
endif()

include_directories(
        ${TOP}/third_party/live555/liveMedia/include/
        ${TOP}/third_party/live555/groupsock/include/
        ${TOP}/third_party/live555/UsageEnvironment/include/
        ${TOP}/third_party/live555/BasicUsageEnvironment/include/
        ${TOP}/third_party/live555/mediaServer/
)
include_directories(${common_inc_dirs} ${libsophon_inc_dirs} ${ffmpeg_inc_dirs} ${opencv_inc_dirs}
)
link_directories(${ffmpeg_link_dirs} ${libsophon_link_dirs} ${opencv_link_dirs})

#set(src_files test.cpp)
add_executable(video_stitch_demo ${src_files})


message(STATUS ${PROJECT_SOURCE_DIR}/examples/video_stitch/rtsp/live555/liveMedia/)
target_link_libraries(video_stitch_demo 
    bmutility
    live555
    ${opencv_link_libs} ${ffmpeg_link_libs} ${libsophon_link_libs} bmtracker jsoncpp pthread
)
