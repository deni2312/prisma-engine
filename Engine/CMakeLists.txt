cmake_minimum_required(VERSION 3.8.0)

project(PrismaEngineDll VERSION 0.1.0)

add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)

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

set(DILIGENT_BUILD_SAMPLES OFF)
set(DILIGENT_INSTALL_SAMPLES OFF)


add_subdirectory("../DiligentEngine" build)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES "include/engine.h" "src/engine.cpp" "include/Components/CloudComponent.h" "src/Components/CloudComponent.cpp" "include/Components/InstancingComponent.h" "src/Components/InstancingComponent.cpp" "include/Components/RegisterCreator.h" "src/Components/RegisterCreator.cpp" src/Pipelines/PipelineSSR.cpp include/Pipelines/PipelineSSR.h src/Components/Component.cpp include/Components/Component.h src/Components/PhysicsMeshComponent.cpp include/Components/PhysicsMeshComponent.h src/Handlers/ComponentsHandler.cpp include/Handlers/ComponentsHandler.h)

add_library(PrismaEngineDll STATIC ${SOURCES} "include/SceneObjects/Node.h" "src/SceneObjects/Node.cpp" "include/SceneData/SceneExporterLayout.h"  "include/SceneData/SceneExporter.h" "src/SceneData/SceneExporter.cpp" "include/SceneData/SceneLoader.h" "src/SceneData/SceneLoader.cpp" "include/Containers/Texture.h" "src/Containers/Texture.cpp" "include/SceneObjects/Camera.h" "src/SceneObjects/Camera.cpp" "include/SceneObjects/Mesh.h" "src/SceneObjects/Mesh.cpp" "include/Pipelines/PipelineForward.h" "src/Pipelines/PipelineForward.cpp" "include/Helpers/Settings.h" "include/Components/MaterialComponent.h" "src/Components/MaterialComponent.cpp" "include/SceneObjects/Light.h" "include/GlobalData/Defines.h" "include/Handlers/LightHandler.h" "src/Handlers/LightHandler.cpp" "include/Helpers/NodeHelper.h" "src/Helpers/NodeHelper.cpp" "include/SceneData/SceneData.h" "include/Helpers/ScenePrinter.h" "src/Helpers/ScenePrinter.cpp" "include/Handlers/MeshHandler.h" "src/Handlers/MeshHandler.cpp" "include/GlobalData/PrismaFunc.h" "src/GlobalData/PrismaFunc.cpp" "include/SceneData/MeshIndirect.h" "src/SceneData/MeshIndirect.cpp" "include/Pipelines/PipelineFullScreen.h" "src/Pipelines/PipelineFullScreen.cpp" "include/Pipelines/PipelineDeferred.h" "src/Pipelines/PipelineDeferred.cpp" "include/GlobalData/GlobalData.h" "include/GlobalData/Keys.h" "include/Pipelines/PipelineHandler.h" "src/Pipelines/PipelineHandler.cpp" "include/Helpers/SettingsLoader.h" "src/Helpers/SettingsLoader.cpp" "include/Pipelines/PipelineSkybox.h" "src/Pipelines/PipelineSkybox.cpp" "src/GlobalData/GlobalData.cpp" "include/Pipelines/PipelineDIffuseIrradiance.h" "src/Pipelines/PipelineDiffuseIrradiance.cpp" "include/Pipelines/PipelinePrefilter.h" "src/Pipelines/PipelinePrefilter.cpp" "include/Pipelines/PipelineLUT.h" "src/Pipelines/PipelineLUT.cpp" "include/Pipelines/PipelineOmniShadow.h" "src/Pipelines/PipelineOmniShadow.cpp" "include/Helpers/PrismaRender.h" "src/Helpers/PrismaRender.cpp" "include/Pipelines/GenericShadow.h" "include/Physics/Physics.h" "src/Physics/Physics.cpp" "include/Physics/Line.h" "src/Physics/Line.cpp" "include/Physics/DrawDebugger.h" "src/Physics/DrawDebugger.cpp" "include/Helpers/PrismaMath.h" "include/Postprocess/Postprocess.h" "src/Postprocess/Postprocess.cpp" "include/Postprocess/PostprocessEffect.h" "src/Postprocess/PostprocessEffect.cpp" "include/Pipelines/PipelineCSM.h" "src/Pipelines/PipelineCSM.cpp"  "include/Helpers/ClusterCalculation.h" "src/Helpers/ClusterCalculation.cpp" "include/Helpers/TimeCounter.h" "include/SceneData/Animator.h" "src/SceneData/Animator.cpp" "include/SceneData/Animation.h" "src/SceneData/Animation.cpp" "include/SceneObjects/AnimatedMesh.h" "src/SceneObjects/AnimatedMesh.cpp" "include/SceneData/Bone.h" "src/SceneData/Bone.cpp" "include/Handlers/AnimationHandler.h" "src/Handlers/AnimationHandler.cpp" "include/GlobalData/CacheScene.h" "src/GlobalData/CacheScene.cpp" "include/Pipelines/PipelinePrePass.h" "src/Pipelines/PipelinePrePass.cpp" "include/GlobalData/EngineSettings.h" "include/Pipelines/PipelineSSAO.h" "src/Pipelines/PipelineSSAO.cpp" "include/SceneObjects/Sprite.h" "src/SceneObjects/Sprite.cpp" "../UserEngine/Components/include/TerrainComponent.h" "../UserEngine/Components/src/TerrainComponent.cpp" "include/Components/CullingComponent.h" "src/Components/CullingComponent.cpp"   "src/Helpers/TimeCounter.cpp" "include/Helpers/Shadinclude.h" "include/Physics/PhysicsData.h" "include/Pipelines/PipelineDeferredForward.h" "src/Pipelines/PipelineDeferredForward.cpp" "include/SceneData/ComponentTypeLayout.h" "include/Handlers/LoadingHandler.h" "src/Handlers/LoadingHandler.cpp" "include/Pipelines/PipelineCulling.h" "src/Pipelines/PipelineCulling.cpp" "include/Helpers/StringHelper.h" "include/Helpers/VectorHelper.h" "include/Helpers/Logger.h" "src/Helpers/Logger.cpp" "src/Helpers/StringHelper.cpp" "src/Helpers/VectorHelper.cpp" "include/Helpers/FrustumCulling.h" "include/Helpers/Interpolator.h" "src/Helpers/Interpolator.cpp" "include/Helpers/AreaHandler.h"  "src/Helpers/AreaHandler.cpp" "include/Helpers/WindowsHelper.h" "src/Helpers/WindowsHelper.cpp" "include/Components/DecalComponent.h" "src/Components/DecalComponent.cpp" "include/GlobalData/Platform.h" "include/GlobalData/GlobalShaderNames.h" "include/Pipelines/PipelineSkyboxRenderer.h" "src/Pipelines/PipelineSkyboxRenderer.cpp" "include/Handlers/OmniShadowHandler.h" "src/Handlers/OmniShadowHandler.cpp" "include/Handlers/CSMHandler.h" "src/Handlers/CSMHandler.cpp" "include/Pipelines/PipelineRayTracing.h" "src/Pipelines/PipelineRayTracing.cpp" "include/Handlers/TLASHandler.h" "src/Handlers/TLASHandler.cpp" "include/Pipelines/PipelineBlitRT.h" "src/Pipelines/PipelineBlitRT.cpp"  "include/Helpers/Blit.h" "src/Helpers/Blit.cpp" "include/Pipelines/PipelineSoftwareRT.h" "src/Pipelines/PipelineSoftwareRT.cpp" "include/Helpers/BVHHelper.h" "src/Helpers/BVHHelper.cpp" "include/Pipelines/PipelineForwardTransparent.h" "src/Pipelines/PipelineForwardTransparent.cpp" "include/Helpers/FPSCounter.h" "src/Helpers/FPSCounter.cpp" "include/Components/RenderComponent.h")

target_include_directories(PrismaEngineDll PUBLIC ${JoltPhysics_SOURCE_DIR}/.. "../DiligentEngine/DiligentCore" "../DiligentEngine/DiligentTools" "include/")

find_package(nlohmann_json CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

target_link_libraries(PrismaEngineDll PRIVATE Jolt glfw glm::glm assimp::assimp nlohmann_json::nlohmann_json Diligent-GraphicsEngineD3D11-static
    Diligent-GraphicsEngineOpenGL-static
    Diligent-GraphicsEngineD3D12-static
    Diligent-GraphicsEngineVk-static Diligent-Imgui Diligent-TextureLoader Diligent-HLSL2GLSLConverterLib)