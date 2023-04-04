cmake_minimum_required(VERSION 3.10.2)

# FFMPEG
set(FFMPEG_PATH ${CMAKE_SOURCE_DIR}/libs/ffmpeg/${ANDROID_ABI})
add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${FFMPEG_PATH}/libavformat.so)
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${FFMPEG_PATH}/libavcodec.so)
add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${FFMPEG_PATH}/libavutil.so)
add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${FFMPEG_PATH}/libavfilter.so)
add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${FFMPEG_PATH}/libswscale.so)
add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${FFMPEG_PATH}/libswresample.so)

# SRC
set(SRC_HEADER
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/ffmpeg
        ${CMAKE_SOURCE_DIR}/include/libavformat
        ${CMAKE_SOURCE_DIR}/include/libavcodec
        ${CMAKE_SOURCE_DIR}/include/libavutil
        ${CMAKE_SOURCE_DIR}/include/libavfilter
        ${CMAKE_SOURCE_DIR}/include/libswscale
        ${CMAKE_SOURCE_DIR}/include/libswresample
        ${CMAKE_SOURCE_DIR}/src
        )
set(SRC_CPPLUS
        ${CMAKE_SOURCE_DIR}/editor_jni.cpp
        ${CMAKE_SOURCE_DIR}/src/timeline.cpp
        ${CMAKE_SOURCE_DIR}/src/repack.cpp
        ${CMAKE_SOURCE_DIR}/src/recode.cpp
        ${CMAKE_SOURCE_DIR}/src/filtering.cpp
        )
include_directories(${SRC_HEADER})
add_library(aveditor SHARED ${SRC_CPPLUS})
# link libs
target_link_libraries(aveditor
        avformat
        avcodec
        avfilter
        avutil
        swscale
        swresample
        EGL
        GLESv3
        OpenSLES
        android
        log
        )

