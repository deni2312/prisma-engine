#include <iostream>
#include "UserEngine/include/UserEngine.h"

int main()
{
	auto userEngine = std::make_shared<UserEngine>();
	Prisma::Engine::getInstance().run();
}
