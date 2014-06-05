// AppConnection.h : Handles connection to other applications.
// A connected app is started by AutoSave and can be sent keyboard input.
// For information on thrown exceptions, see below.

#pragma once

#include "stdafx.h"
#include "CommandLineParser.h"
#include "OleUtils.h"
#include "AutoSaveException.h"

class AppConnection
{
public:
	// constructors and destructor
	AppConnection() : m_hProcess(0), m_processId(0) {}
	~AppConnection() { disconnect(); }

	// copy constructors
	AppConnection(const AppConnection& ac);
	AppConnection& operator=(const AppConnection& ac);

	// move constructors
	AppConnection(AppConnection&& ac);
	AppConnection& operator=(AppConnection&& ac);

	// May throw AutoSaveException on failure.
	void connect(const vector<wstring>& args);
	void connect(const wstring& file);
	void disconnect();

	// Throws std::runtime_error if starting this file would result in recursion.
	// Throws AutoSaveException or OleException on failure.
	static void throwIfStartingSelf(const wstring& startedFile);

	// Getters.
	inline DWORD getProcessId() const { return m_processId; }
	inline bool isConnected() const { return m_processId != 0; }
	bool isConnectionAlive() const {
		return isConnected() && getExitCode(m_hProcess) == STILL_ACTIVE;
	}
	inline DWORD waitForInputIdle(DWORD timeout) const {
		return WaitForInputIdle(m_hProcess, timeout);
	}

	vector<HWND> getConnectedWindows() const;

private:
	static DWORD getExitCode(HANDLE hProcess);

	static BOOL CALLBACK getConnectedWindowsEnumProc(HWND hwnd, LPARAM lParam);
	struct EnumProcArgs {
		DWORD expectedProcessId;
		vector<HWND> matchingWindows;
	};

	static HANDLE shellExecute(const wstring& file, const wstring& argLine);
	static wstring findExecutable(const wstring& documentPath);

	DWORD m_processId;
	HANDLE m_hProcess;
};

