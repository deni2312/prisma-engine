cmake_minimum_required(VERSION 3.8.0)

project(PrismaEngineDll VERSION 0.1.0)

include("..\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")

include(FetchContent)

FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics"
        GIT_TAG "0a7d2501b33e1251101392d2be403c551ce5f963"
		SOURCE_SUBDIR "Build"
)
FetchContent_MakeAvailable(JoltPhysics)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES "src/engine.cpp" "include/Components/CloudComponent.h" "src/Components/CloudComponent.cpp" ../GUI/src/MeshInfo.cpp ../GUI/include/MeshInfo.h src/Containers/VAO.cpp include/Containers/VAO.h src/Containers/VBO.cpp include/Containers/VBO.h src/Containers/EBO.cpp include/Containers/EBO.h ../GUI/src/ImGuiTabs.cpp ../GUI/include/ImGuiTabs.h src/Pipelines/PipelineSSR.cpp include/Pipelines/PipelineSSR.h src/Components/Component.cpp include/Components/Component.h src/Components/PhysicsMeshComponent.cpp include/Components/PhysicsMeshComponent.h src/Handlers/ComponentsHandler.cpp include/Handlers/ComponentsHandler.h ../GUI/src/FolderView.cpp ../GUI/include/FolderView.h)

add_library(PrismaEngineDll STATIC ${SOURCES} "include/SceneObjects/Node.h" "src/SceneObjects/Node.cpp" "include/SceneData/SceneExporterLayout.h" "include/Helpers/Shader.h" "include/SceneData/SceneExporter.h" "src/SceneData/SceneExporter.cpp" "include/Helpers/Shader.h" "include/Helpers/stb_image.h" "include/SceneData/SceneLoader.h" "src/SceneData/SceneLoader.cpp" "include/Containers/Texture.h" "src/Containers/Texture.cpp" "include/SceneObjects/Camera.h" "src/SceneObjects/Camera.cpp" "include/SceneObjects/Mesh.h" "src/SceneObjects/Mesh.cpp" "include/Pipelines/PipelineForward.h" "src/Pipelines/PipelineForward.cpp" "include/Helpers/Settings.h" "include/Components/MaterialComponent.h" "src/Components/MaterialComponent.cpp" "include/SceneObjects/Light.h" "../GUI/include/ImGuiDebug.h" "../GUI/src/ImguiDebug.cpp" "../GUI/include/SettingsTab.h" "../GUI/src/SettingsTab.cpp" "../GUI/include/ImGuiCamera.h" "../GUI/src/ImguiCamera.cpp" "include/Containers/Ubo.h" "include/GlobalData/Defines.h" "include/Handlers/LightHandler.h" "src/Handlers/LightHandler.cpp" "include/Helpers/NodeHelper.h" "src/Helpers/NodeHelper.cpp" "include/SceneData/SceneData.h" "include/Helpers/ScenePrinter.h" "src/Helpers/ScenePrinter.cpp" "include/Containers/SSBO.h" "include/Handlers/MeshHandler.h" "src/Handlers/MeshHandler.cpp" "include/GlobalData/PrismaFunc.h" "src/GlobalData/PrismaFunc.cpp" "include/SceneData/MeshIndirect.h" "src/SceneData/MeshIndirect.cpp" "include/Containers/FBO.h" "src/Containers/FBO.cpp" "include/Pipelines/PipelineFullScreen.h" "src/Pipelines/PipelineFullScreen.cpp" "include/Pipelines/PipelineDeferred.h" "src/Pipelines/PipelineDeferred.cpp" "include/GlobalData/GlobalData.h" "include/GlobalData/Keys.h" "src/Containers/SSBO.cpp" "src/Containers/UBO.cpp" "include/Pipelines/PipelineHandler.h" "src/Pipelines/PipelineHandler.cpp" "include/Helpers/SettingsLoader.h" "src/Helpers/SettingsLoader.cpp" "include/Pipelines/PipelineSkybox.h" "src/Pipelines/PipelineSkybox.cpp" "src/GlobalData/GlobalData.cpp" "include/Pipelines/PipelineDIffuseIrradiance.h" "src/Pipelines/PipelineDiffuseIrradiance.cpp" "include/Pipelines/PipelinePrefilter.h" "src/Pipelines/PipelinePrefilter.cpp" "include/Pipelines/PipelineLUT.h" "src/Pipelines/PipelineLUT.cpp" "include/Pipelines/PipelineOmniShadow.h" "src/Pipelines/PipelineOmniShadow.cpp" "include/Helpers/PrismaRender.h" "src/Helpers/PrismaRender.cpp" "include/Pipelines/GenericShadow.h" "include/Physics/Physics.h" "src/Physics/Physics.cpp" "include/Physics/Line.h" "src/Physics/Line.cpp" "include/Physics/DrawDebugger.h" "src/Physics/DrawDebugger.cpp" "include/Helpers/GarbageCollector.h" "src/Helpers/GarbageCollector.cpp" "include/Helpers/PrismaMath.h" "include/Postprocess/Postprocess.h" "src/Postprocess/Postprocess.cpp" "include/Postprocess/PostprocessEffect.h" "src/Postprocess/PostprocessEffect.cpp" "include/Pipelines/PipelineCSM.h" "src/Pipelines/PipelineCSM.cpp"  "include/Helpers/ClusterCalculation.h" "src/Helpers/ClusterCalculation.cpp" "include/Helpers/TimeCounter.h" "include/SceneData/Animator.h" "src/SceneData/Animator.cpp" "include/SceneData/Animation.h" "src/SceneData/Animation.cpp" "include/SceneObjects/AnimatedMesh.h" "src/SceneObjects/AnimatedMesh.cpp" "include/SceneData/Bone.h" "src/SceneData/Bone.cpp" "include/Handlers/AnimationHandler.h" "src/Handlers/AnimationHandler.cpp" "include/GlobalData/CacheScene.h" "src/GlobalData/CacheScene.cpp" "include/Pipelines/PipelinePrePass.h" "src/Pipelines/PipelinePrePass.cpp" "include/GlobalData/EngineSettings.h" "include/Pipelines/PipelineSSAO.h" "src/Pipelines/PipelineSSAO.cpp" "include/SceneObjects/Sprite.h" "src/SceneObjects/Sprite.cpp" "../UserEngine/Components/include/TerrainComponent.h" "../UserEngine/Components/src/TerrainComponent.cpp" "include/Components/CullingComponent.h" "src/Components/CullingComponent.cpp"   "src/Helpers/TimeCounter.cpp" "include/Helpers/Shadinclude.h" "include/Physics/PhysicsData.h" "include/Pipelines/PipelineDeferredForward.h" "src/Pipelines/PipelineDeferredForward.cpp" "include/SceneData/ComponentTypeLayout.h" "include/Handlers/LoadingHandler.h" "src/Handlers/LoadingHandler.cpp" "include/Pipelines/PipelineCulling.h" "src/Pipelines/PipelineCulling.cpp" "include/Helpers/StringHelper.h")

target_include_directories(PrismaEngineDll PUBLIC ${JoltPhysics_SOURCE_DIR}/..)


find_package(nlohmann_json CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)

target_link_libraries(PrismaEngineDll PRIVATE Jolt GLEW::GLEW glfw glm::glm assimp::assimp imgui::imgui nlohmann_json::nlohmann_json imguizmo::imguizmo implot::implot)
