
add_compile_options(-Wno-deprecated-declarations)

# find_package(exiv2 REQUIRED)

add_executable(retinaface_demo main.cpp worker.cpp retinaface.cpp)

if (USE_QTGUI)
    target_link_libraries(retinaface_demo bmutility ${opencv_link_libs} ${ffmpeg_link_libs} ${libsophon_link_libs} bmgui jsoncpp) # add exiv2 if needed
else()
    target_link_libraries(retinaface_demo bmutility ${opencv_link_libs} ${ffmpeg_link_libs} ${libsophon_link_libs} jsoncpp pthread) # add exiv2 if needed
endif()

#target_compile_definitions(retinaface_demo PUBLIC USE_EXIV2=1)