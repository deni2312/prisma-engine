cmake_minimum_required(VERSION 3.8.0)
project(Prisma-client VERSION 0.1.0)
add_subdirectory(engine)
add_subdirectory(gui)
set(CMAKE_CXX_STANDARD 17)
add_executable(Prisma-client "main.cpp" "UserEngine/include/UserEngine.h" "UserEngine/src/UserEngine.cpp" "UserEngine/include/PlayerController.h" "UserEngine/src/PlayerController.cpp" "UserEngine/include/ParticleController.h" "UserEngine/src/ParticleController.cpp")
find_package(glm CONFIG REQUIRED)


target_link_libraries(Prisma-client PRIVATE PrismaEngineDll PrismaGUI glm::glm)
copy_required_dlls(Prisma-client)

