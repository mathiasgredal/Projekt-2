cmake_minimum_required(VERSION 3.16)

project(untitled LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include tensorflow lite(possibly add automatic download)
add_subdirectory("extern/tensorflow/tensorflow/lite" "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite" EXCLUDE_FROM_ALL)

# Include Qt5
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)


set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/objectdetector.h
        src/objectdetector.cpp
)

add_executable(untitled ${PROJECT_SOURCES})
target_link_libraries(untitled PRIVATE
    tensorflow-lite
    ${CMAKE_DL_LIBS} Qt5::Widgets)
