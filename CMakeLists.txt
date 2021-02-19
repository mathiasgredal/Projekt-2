cmake_minimum_required(VERSION 3.16)

project(Projekt_2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMake Package Manager
include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)

# Add TFLite
CPMAddPackage(
        NAME tensorflow
        GIT_REPOSITORY https://github.com/tensorflow/tensorflow.git
        GIT_TAG "v2.4.1"
        DOWNLOAD_ONLY
)
### Add only tensorflow lite
add_subdirectory("${tensorflow_SOURCE_DIR}/tensorflow/lite" "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite" EXCLUDE_FROM_ALL)

# Add LibAV aka. FFMPEG
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
            libavdevice
            libavfilter
            libavformat
            libavcodec
            libswresample
            libswscale
            libavutil)

# Add multithreading
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Add Qt5
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 CONFIG REQUIRED Core Widgets Gui SerialPort Multimedia)

set(SOURCES
        Sources/main.cpp
        Sources/mainwindow.cpp
        Sources/videoplayer.cpp
        Sources/objectdetector.cpp
)

set(HEADERS
        Headers/mainwindow.h
        Headers/videoplayer.h
        Headers/objectdetector.h
)

set(FORMS
        Forms/mainwindow.ui
)

add_executable(Projekt_2 ${SOURCES} ${HEADERS} ${FORMS})
target_link_libraries(Projekt_2 PRIVATE
    tensorflow-lite ${CMAKE_DL_LIBS}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::SerialPort
    Qt5::Multimedia
    PkgConfig::LIBAV
    Threads::Threads)
include_directories(Projekt_2 PRIVATE Headers)
