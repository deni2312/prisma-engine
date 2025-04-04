#include "UserEngine/include/UserEngine.h"
#include "GUI/include/ImGuiDebug.h"

int main(int argc, const char* const* argv)
{
	//_CrtSetBreakAlloc(9694);
	auto userEngine = std::make_shared<UserEngine>();
	Prisma::Engine::getInstance().setUserEngine(userEngine);
	Prisma::Engine::getInstance().setGuiData(Prisma::ImguiDebug::getInstance().handlers());
	Prisma::Engine::getInstance().run();
	return 0;
}
