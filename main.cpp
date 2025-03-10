#include <iostream>
#include "UserEngine/include/UserEngine.h"
#include <Windows.h>

int main(int argc, const char* const* argv)
{
	auto userEngine = std::make_shared<UserEngine>();
	Prisma::Engine::getInstance().setUserEngine(userEngine);
	Prisma::Engine::getInstance().setGuiData(Prisma::ImguiDebug::getInstance().handlers());
	Prisma::Engine::getInstance().run();
	return 0;
}
