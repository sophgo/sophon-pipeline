cmake_minimum_required(VERSION 3.2)


add_compile_options(-Wno-deprecated-declarations)

if(USE_DEBUG)
    add_definitions(-DUSE_DEBUG=1)
endif()

set(src_files main.cpp worker.cpp)
add_definitions(-DHAVE_FACEDETECT_MODULE=1)
list(APPEND src_files face_detector.cpp face_extract.cpp resnet50.cpp)

include_directories(${common_inc_dirs} ${libsophon_inc_dirs} ${ffmpeg_inc_dirs} ${opencv_inc_dirs})
link_directories(${ffmpeg_link_dirs} ${libsophon_link_dirs} ${opencv_link_dirs})


add_executable(cvs20 ${src_files})
message("====================================")
message("====================================")
message("path: ${opencv_link_libs}")
if (USE_QTGUI)
    target_link_libraries(cvs20 bmutility ${opencv_link_libs} ${ffmpeg_link_libs} ${libsophon_link_libs} bmtracker bmgui jsoncpp)
else()
    target_link_libraries(cvs20 bmutility ${opencv_link_libs} ${ffmpeg_link_libs} ${libsophon_link_libs} bmtracker jsoncpp pthread)
endif()


