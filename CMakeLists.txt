cmake_minimum_required(VERSION 3.27)
project(graphics_sandbox)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets REQUIRED)

# Find windeployqt for release/debug builds on windows only
if (WIN32)
    get_filename_component(WINDEPLOYQT_DIR ${WINDEPLOYQT_EXECUTABLE} DIRECTORY)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(WINDEPLOYQT ${WINDEPLOYQT_DIR}/windeployqt.debug.bat)
    else()
        set(WINDEPLOYQT ${WINDEPLOYQT_DIR}/windeployqt.exe)
    endif()

    message("using windeployqt: ${WINDEPLOYQT}")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(sandbox)