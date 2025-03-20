cmake_minimum_required(VERSION 3.8.0)
project(Prisma-client VERSION 0.1.0)
add_subdirectory(engine)
set(CMAKE_CXX_STANDARD 17)
add_executable(Prisma-client "main.cpp" "UserEngine/include/UserEngine.h" "UserEngine/src/UserEngine.cpp" "GUI/include/ImGuiDebug.h" "GUI/src/ImGuiDebug.cpp" "GUI/include/ImGuiStyle.h" "GUI/include/ImGuiCamera.h" "GUI/src/ImGuiCamera.cpp"  "GUI/include/FolderView.h" "GUI/src/FolderView.cpp" "GUI/include/ImGuiLog.h" "GUI/src/ImGuiLog.cpp" "GUI/include/ImGuiHelper.h" "GUI/src/ImGuiHelper.cpp" "GUI/include/ImGuiTabs.h" "GUI/src/ImGuiTabs.cpp" "GUI/include/ImGuiKey.h" "GUI/src/AddingMenu.cpp" "GUI/include/AddingMenu.h" "GUI/include/NodeCreator.h" "GUI/src/NodeCreator.cpp" "GUI/include/ScenePipeline.h" "GUI/src/ScenePipeline.cpp" "GUI/include/NodeViewer.h" "GUI/src/NodeViewer.cpp" "GUI/include/MeshInfo.h" "GUI/src/MeshInfo.cpp" "GUI/include/LightInfo.h" "GUI/src/LightInfo.cpp")
find_package(glm CONFIG REQUIRED)


target_link_libraries(Prisma-client PRIVATE PrismaEngineDll glm::glm)
copy_required_dlls(Prisma-client)

