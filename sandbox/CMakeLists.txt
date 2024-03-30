project(sandbox)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(sandbox
        main.cpp
        UI/MainWindow/MainWindow.cpp
        UI/MainWindow/MainWindow.h
        UI/MainWindow/MainWindow.ui
        Model/Hierarchy/Scene.h
        Model/Hierarchy/Scene.cpp
        Model/Hierarchy/Object.cpp
        Model/Hierarchy/Object.h
        Model/Components/ComponentsRegistry.h
        Common/Common.h
        Model/Settings/ViewSettings.h
        UI/ObjectEditor/ObjectEditor.cpp
        UI/ObjectEditor/ObjectEditor.h
        UI/ObjectEditor/ObjectEditor.ui
        UI/SceneBrowser/SceneBrowser.cpp
        UI/SceneBrowser/SceneBrowser.h
        UI/SceneBrowser/SceneBrowser.ui
        UI/View/OpenGL/OpenGLView.cpp
        UI/View/OpenGL/OpenGLView.h
        Renderer/OpenGL/OpenGLRenderer.cpp
        Renderer/OpenGL/OpenGLRenderer.h
        Common/ViewRegistrar.h)

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
