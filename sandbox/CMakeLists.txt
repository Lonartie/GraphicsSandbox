project(sandbox)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_add_executable(sandbox
        main.cpp
        UI/MainWindow/MainWindow.cpp
        UI/MainWindow/MainWindow.h
        UI/MainWindow/MainWindow.ui
        Model/Hierarchy/Scene.h
        Model/Hierarchy/Scene.cpp
        Model/Hierarchy/Object.cpp
        Model/Hierarchy/Object.h
        Model/Components/TransformComponent.h
        Model/Components/CameraComponent.h
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
        UI/View/ViewBase.h
        Model/Components/Component.h
        Model/Model.h
        UI/Components/ComponentsView.h
        UI/Components/CameraComponentView/CameraComponentView.cpp
        UI/Components/CameraComponentView/CameraComponentView.h
        UI/Components/CameraComponentView/CameraComponentView.ui
        UI/Components/TransformComponentView/TransformComponentView.cpp
        UI/Components/TransformComponentView/TransformComponentView.h
        UI/Components/TransformComponentView/TransformComponentView.ui
        Stylesheets/Stylesheets.qrc
        Model/Components/MeshComponent.h
        UI/Components/MeshComponentView/MeshComponentView.cpp
        UI/Components/MeshComponentView/MeshComponentView.h
        UI/Components/MeshComponentView/MeshComponentView.ui
        UI/Components/MeshComponentView/MeshOpenGLWidget/MeshOpenGLWidget.cpp
        UI/Components/MeshComponentView/MeshOpenGLWidget/MeshOpenGLWidget.h
        Common/ShaderProvider.cpp
        Common/ShaderProvider.h
        Model/Components/MaterialComponent.h
)

target_link_libraries(sandbox
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
)

if (APPLE)
    target_link_libraries(sandbox PRIVATE "-framework OpenGL")
endif ()

set_target_properties(sandbox PROPERTIES
#        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
)

if (WIN32)
    add_custom_command(TARGET sandbox POST_BUILD
            COMMAND Qt6::windeployqt
            ARGS $<TARGET_FILE:sandbox>
    )
endif ()

# folder which shall be copied to the build directory
set(SHADER_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/Renderer/OpenGL/Shaders)

add_custom_command(TARGET sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SHADER_FOLDER}
        $<TARGET_FILE_DIR:sandbox>/Shaders
)
