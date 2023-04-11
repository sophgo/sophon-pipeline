cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wno-deprecated-declarations)

set(src_files main.cpp worker.cpp ppyoloe.cpp)

if(USE_DEBUG)
    add_definitions(-DUSE_DEBUG=1)
endif()

add_executable(ppyoloe_demo ${src_files})
if (USE_QTGUI)
    target_link_libraries(ppyoloe_demo bmutility ${opencv_link_libs} ${ffmpeg_link_libs} ${libsophon_link_libs} bmgui bmtracker jsoncpp)
else()
    target_link_libraries(ppyoloe_demo bmutility ${opencv_link_libs} ${ffmpeg_link_libs} ${libsophon_link_libs} jsoncpp bmtracker pthread)
endif()
