cmake_minimum_required(VERSION 3.8.0)
project(PrismaGUI VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
add_library(PrismaGUI STATIC "include/ImGuiDebug.h" "src/ImGuiDebug.cpp" "include/ImGuiStyle.h" "include/ImGuiCamera.h" "src/ImGuiCamera.cpp"  "include/FolderView.h" "src/FolderView.cpp" "include/ImGuiLog.h" "src/ImGuiLog.cpp" "include/ImGuiHelper.h" "src/ImGuiHelper.cpp" "include/ImGuiTabs.h" "src/ImGuiTabs.cpp" "include/ImGuiKey.h" "src/AddingMenu.cpp" "include/AddingMenu.h" "include/NodeCreator.h" "src/NodeCreator.cpp" "include/NodeViewer.h" "src/NodeViewer.cpp" "include/MeshInfo.h" "src/MeshInfo.cpp" "include/LightInfo.h" "src/LightInfo.cpp" "include/TextureInfo.h" "src/TextureInfo.cpp" "include/SettingsTab.h" "src/SettingsTab.cpp" "imguizmo/imguizmo.h" "imguizmo/imguizmo.cpp" "include/PixelCapture.h" "src/PixelCapture.cpp" "include/QueryGPU.h" "src/QueryGPU.cpp" "include/Postprocess/Effects.h" "src/Postprocess/Effects.cpp" "include/Postprocess/Bloom.h" "src/Postprocess/Bloom.cpp" "include/Postprocess/Styles.h" "src/Postprocess/Styles.cpp" "include/Postprocess/VolumetricLight.h" "src/Postprocess/VolumetricLight.cpp" "include/Postprocess/VolumetricRays.h" "src/Postprocess/VolumetricRays.cpp" "include/Postprocess/FXAA.h" "src/Postprocess/FXAA.cpp"  )
find_package(glm CONFIG REQUIRED)
target_include_directories(PrismaGUI PUBLIC "../Engine/include/" "include/")

target_link_libraries(PrismaGUI PRIVATE PrismaEngineDll Diligent-Imgui glm::glm Diligent-TextureLoader)
