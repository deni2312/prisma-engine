#include <iostream>
#include "UserEngine/include/UserEngine.h"

int main()
{
	auto userEngine = std::make_shared<UserEngine>();
	Prisma::Engine::getInstance().setUserEngine(userEngine);
	Prisma::Engine::getInstance().setGuiData(Prisma::ImguiDebug::getInstance().handlers());
	Prisma::Engine::getInstance().run();
}
