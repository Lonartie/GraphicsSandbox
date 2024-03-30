project(sandbox)

add_executable(sandbox main.cpp
        MainWindow/mainwindow.cpp
        MainWindow/mainwindow.h
        MainWindow/mainwindow.ui)

target_link_libraries(sandbox
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
)

# Adding custom command to run windeployqt on the sandbox executable
if (WIN32)
    add_custom_command(TARGET sandbox POST_BUILD
        COMMAND echo "Running windeployqt on $<TARGET_FILE:sandbox>"
        COMMAND ${WINDEPLOYQT} $<TARGET_FILE:sandbox>
    )
endif()
