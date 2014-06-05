#include "stdafx.h"
#include "AutoSaveException.h"



AutoSaveException::AutoSaveException(DWORD errorCode)
	: std::runtime_error(""),
	  m_hResult(HRESULT_FROM_WIN32(errorCode)),
	  m_ansiBuffer(NULL),
	  m_wideBuffer(NULL)
{
}

AutoSaveException::~AutoSaveException()
{
	LocalFree((HLOCAL) m_ansiBuffer);
	LocalFree((HLOCAL) m_wideBuffer);
}



LPCSTR AutoSaveException::what()
{
	if (m_ansiBuffer == NULL)
	{
		size_t charCount = FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, m_hResult, LANG_NEUTRAL,
			(char*) &m_ansiBuffer, 0, NULL);
		if (charCount == 0)
		{
			LocalFree((HLOCAL) m_ansiBuffer);
			charCount = FormatMessageA(
				FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_ARGUMENT_ARRAY,
				"Unknown exception (HRESULT value 0x%1!08X!)\r\n", 0,
				LANG_NEUTRAL, (char*) &m_ansiBuffer, 0, (va_list*) &m_hResult);
			if (charCount == 0)
			{
				LocalFree((HLOCAL) m_ansiBuffer);
				m_ansiBuffer = "Unknown exception\r\n";
			}
		}
	}
	return m_ansiBuffer;
}



LPCWSTR AutoSaveException::wcwhat()
{
	if (m_wideBuffer == NULL)
	{
		size_t charCount = FormatMessageW(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, m_hResult, LANG_NEUTRAL,
			(wchar_t*) &m_wideBuffer, 0, NULL);
		if (charCount == 0)
		{
			LocalFree((HLOCAL) m_wideBuffer);
			charCount = FormatMessageW(
				FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_ARGUMENT_ARRAY,
				L"Unknown exception (HRESULT value 0x%1!08X!)\r\n", 0,
				LANG_NEUTRAL, (wchar_t*) &m_wideBuffer, 0, (va_list*) &m_hResult);
			if (charCount == 0)
			{
				LocalFree((HLOCAL) m_wideBuffer);
				m_wideBuffer = L"Unknown exception\r\n";
			}
		}
	}
	return m_wideBuffer;
}



void AutoSaveException::showMessageBox(HWND hwnd, wstring text)
{
	text.append(L"\nThe reason was a ");
	text.append(exceptionType());
	text.append(L". This is the error message returned by the system:\n");
	text.append(wcwhat());
	MessageBox(hwnd, text.data(), APP_NAME, MB_ICONERROR | MB_TASKMODAL);
}



void AutoSaveException::showMessageBox(HWND hwnd, wstring text, std::exception& exc)
{
	text.append(L"\nThe reason was an unspecified exception. "
		L"This is the error message returned by the system:\n");
	text.append(ansiToWide(exc.what()));
	MessageBox(hwnd, text.data(), APP_NAME, MB_ICONERROR | MB_TASKMODAL);
}



wstring AutoSaveException::ansiToWide(LPCSTR ansi)
{
	if (ansi == "")
		return L"No error message specified";

	size_t sizeNeeded = MultiByteToWideChar(
		CP_ACP, MB_USEGLYPHCHARS, ansi, -1, NULL, 0);
	if (sizeNeeded == 0)
		return L"Couldn't retrieve error message";

	std::auto_ptr<wchar_t> wide(new wchar_t[sizeNeeded]);
	size_t result = MultiByteToWideChar(
		CP_ACP, MB_USEGLYPHCHARS, ansi, -1, wide.get(), (int) sizeNeeded);
	if (result == 0)
		return L"Couldn't retrieve error message";

	return wide.get();
}

