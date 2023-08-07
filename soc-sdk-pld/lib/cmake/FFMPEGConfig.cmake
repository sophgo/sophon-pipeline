# ===================================================================================
#  The FFMPEG CMake configuration file
#
#  Usage from an external project:
#    In your CMakeLists.txt, add these lines:
#
#    set(FFMPEG_DIR  /opt/sophon/sophon-ffmpeg-latest/lib/cmake)
#    find_package(FFMPEG REQUIRED)
#    include_directories(${FFMPEG_INCLUDE_DIRS})
#    target_link_libraries(MY_TARGET_NAME ${FFMPEG_LIBS})
#
#    If the library is found then FFMPEG_FOUND is set to TRUE,
#    else set to FALSE.
#
#    This file will define the following variables:
#      - FFMPEG_LIBS                     : The list of all libs for FFMPEG modules.
#      - FFMPEG_INCLUDE_DIRS             : The FFMPEG include directories.
#      - FFMPEG_LIB_DIRS                 : The FFMPEG library directories
#
# ===================================================================================

set(FFMPEG_INCLUDE_DIRS "/opt/sophon/sophon-ffmpeg-latest/include")
set(FFMPEG_LIB_DIRS "/opt/sophon/sophon-ffmpeg-latest/lib")

set(lib_names avcodec avformat avutil avfilter avdevice swscale swresample)
foreach(name ${lib_names})
    set(FFMPEG_FOUND TRUE)
    find_library(the_${name} "${name}" PATHS ${FFMPEG_LIB_DIRS} NO_DEFAULT_PATH)
    if(the_${name})
        list(APPEND FFMPEG_LIBS "${the_${name}}")
    else()
        set(FFMPEG_FOUND FALSE)
        break()
    endif()
endforeach()
