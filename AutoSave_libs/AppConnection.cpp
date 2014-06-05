#include "stdafx.h"
#include "AppConnection.h"


// Copy constructor
AppConnection::AppConnection(const AppConnection& ac)
	: m_processId(ac.m_processId)
{
	if (ac.isConnected())
	{
		// Explicitly duplicate handles for reference count purposes.
		DuplicateHandle(
			GetCurrentProcess(), ac.m_hProcess,
			GetCurrentProcess(), &m_hProcess,
			0, FALSE, DUPLICATE_SAME_ACCESS);
	}
}



// Copy assignment constructor
AppConnection& AppConnection::operator=(const AppConnection& ac)
{
	if (this != &ac)
	{
		// Invalidate old handles before they are lost.
		disconnect();
		m_processId = ac.m_processId;
		if (ac.isConnected())
		{
			// Explicitly duplicate handles for reference count purposes.
			DuplicateHandle(
				GetCurrentProcess(), ac.m_hProcess,
				GetCurrentProcess(), &m_hProcess,
				0, FALSE, DUPLICATE_SAME_ACCESS);
		}
	}
	return *this;
}



// Move constructor
AppConnection::AppConnection(AppConnection&& ac)
	: m_processId(ac.m_processId),
	  m_hProcess(ac.m_hProcess)
{
	// We needn't close the handles since we keep them in this object.
	ac.m_hProcess = 0;
	ac.m_processId = 0;
}



// Move assignment constructor
AppConnection& AppConnection::operator=(AppConnection&& ac)
{
	// Invalidate old handles before they are lost.
	disconnect();
	// We needn't close the handles since we keep them in this object.
	m_hProcess = ac.m_hProcess;
	m_processId = ac.m_processId;
	ac.m_hProcess = 0;
	ac.m_processId = 0;

	return *this;
}



void AppConnection::connect(const vector<wstring>& args)
{
	disconnect();

	const wstring& openedFile = args[0];
	wstring commandLine = CommandLineParser::joinArguments(
		args.begin() + 1, args.end());

	throwIfStartingSelf(openedFile);
	m_hProcess = shellExecute(openedFile, commandLine);
	m_processId = GetProcessId(m_hProcess);
	if (m_processId == 0)
	{
		CloseHandle(m_hProcess);
		throw AutoSaveException();
	}
}



void AppConnection::connect(const wstring& file)
{
	vector<wstring> args = { file };
	connect(args);
}



void AppConnection::disconnect()
{
	if (isConnected())
	{
		CloseHandle(m_hProcess);
		m_hProcess = 0;
		m_processId = 0;
	}
}




HANDLE AppConnection::shellExecute(const wstring& file, const wstring& argLine)
{
	STARTUPINFO si;
	GetStartupInfo(&si);

	SHELLEXECUTEINFO see = { 0 };
	see.cbSize = sizeof(see);
	see.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_LOG_USAGE;
	see.lpVerb = L"open";
	see.lpFile = file.data();
	see.lpParameters = argLine.data();
	see.lpDirectory = NULL;
	see.nShow = si.wShowWindow;

	throwIfZero<AutoSaveException, BOOL>(
		ShellExecuteEx(&see));
	return see.hProcess;
}



void AppConnection::throwIfStartingSelf(const wstring& startedFile)
{
	if (OleUtils::isSelf(findExecutable(startedFile)))
		throw std::runtime_error("AutoSave must not start itself");
}



wstring AppConnection::findExecutable(const wstring& documentPath)
{
	if (OleUtils::isExecutable(documentPath))
		return documentPath;

	TCHAR buffer[MAX_PATH];
	HINSTANCE feResult = FindExecutable(documentPath.data(), NULL, buffer);
	if ((int) feResult <= 32)
		throw AutoSaveException();
	return buffer;
}



DWORD AppConnection::getExitCode(HANDLE hProcess)
{
	DWORD exitCode = 0;
	return GetExitCodeProcess(hProcess, &exitCode) == FALSE ? 0 : exitCode;
}



vector<HWND> AppConnection::getConnectedWindows() const
{
	EnumProcArgs args = { getProcessId(), {} };
	EnumWindows(getConnectedWindowsEnumProc, (LPARAM) &args);
	return args.matchingWindows;
}

BOOL CALLBACK AppConnection::getConnectedWindowsEnumProc(
	HWND hwnd, LPARAM lParam)
{
	auto pArgs = (EnumProcArgs*) lParam;
	DWORD windowProcessId;
	GetWindowThreadProcessId(hwnd, &windowProcessId);
	
	if (windowProcessId == pArgs->expectedProcessId && GetParent(hwnd) == 0)
		pArgs->matchingWindows.push_back(hwnd);

	return TRUE;
}





