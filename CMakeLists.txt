cmake_minimum_required(VERSION 3.8.0)
project(Prisma-client VERSION 0.1.0)
add_subdirectory(engine)
set(CMAKE_CXX_STANDARD 17)
add_executable(Prisma-client "main.cpp" "UserEngine/include/UserEngine.h" "UserEngine/src/UserEngine.cpp" "GUI/include/ImGuiStyle.h" "GUI/include/Postprocess/Effects.h" "GUI/src/Postprocess/Effects.cpp" "GUI/include/LightInfo.h" "GUI/src/LightInfo.cpp" "GUI/include/TextureInfo.h" "GUI/src/TextureInfo.cpp" "GUI/include/PixelCapture.h" "GUI/src/PixelCapture.cpp" "UserEngine/include/PlayerController.h" "UserEngine/src/PlayerController.cpp" "GUI/include/SettingsTab.h" "GUI/src/SettingsTab.cpp" "GUI/include/PlotFPS.h" "GUI/src/PlotFPS.cpp" "UserEngine/include/ParticleController.h" "UserEngine/src/ParticleController.cpp" "GUI/include/NodeViewer.h" "GUI/src/NodeViewer.cpp" "UserEngine/Components/include/GrassRenderer.h" "UserEngine/Components/src/GrassRenderer.cpp" "GUI/include/NodeCreator.h" "GUI/src/NodeCreator.cpp" "GUI/include/AddingMenu.h" "GUI/src/AddingMenu.cpp" "Engine/include/Components/RegisterComponent.h" "Engine/include/Components/RegisterCreator.h" "Engine/src/Components/RegisterCreator.cpp" "GUI/include/ImGuiLog.h" "GUI/src/ImGuiLog.cpp" "UserEngine/Components/include/ShockwaveComponent.h" "UserEngine/Components/src/ShockwaveComponent.cpp" "UserEngine/Components/Postprocess/include/Shockwave.h" "UserEngine/Components/Postprocess/src/Shockwave.cpp")
find_package(glm CONFIG REQUIRED)

target_link_libraries(Prisma-client PRIVATE PrismaEngineDll glm::glm)
