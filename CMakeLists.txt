cmake_minimum_required(VERSION 3.8.0)
project(Prisma-client VERSION 0.1.0)
add_subdirectory(engine)
set(CMAKE_CXX_STANDARD 17)
add_executable(Prisma-client "main.cpp" "UserEngine/include/UserEngine.h" "UserEngine/src/UserEngine.cpp" "GUI/include/ImGuiStyle.h" "GUI/include/Postprocess/Effects.h" "GUI/src/Postprocess/Effects.cpp" "GUI/include/LightInfo.h" "GUI/src/LightInfo.cpp" "GUI/include/TextureInfo.h" "GUI/src/TextureInfo.cpp")
find_package(glm CONFIG REQUIRED)

target_link_libraries(Prisma-client PRIVATE PrismaEngineDll glm::glm)
