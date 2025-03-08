#include <iostream>
#include "UserEngine/include/UserEngine.h"
#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	auto userEngine = std::make_shared<UserEngine>();
	Prisma::Engine::getInstance().setUserEngine(userEngine);
	Prisma::Engine::getInstance().windowsData({ hInstance ,nCmdShow });
	Prisma::Engine::getInstance().run();
	return 0;
}
