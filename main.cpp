#include "UserEngine/include/UserEngine.h"
#include "GUI/include/ImGuiDebug.h"

int main(int argc, const char* const* argv) {
    //_CrtSetBreakAlloc(171253);
    auto userEngine = std::make_shared<UserEngine>();
    Prisma::Engine::getInstance().setUserEngine(userEngine);
    Prisma::Engine::getInstance().setGuiData(Prisma::GUI::ImguiDebug::getInstance().handlers());
    Prisma::Engine::getInstance().run();
    return 0;
}