// Configuration.h : Wraps all options surrounding AutoSave.
// Throws only if the wrapped functions throw. (E.g. loadFromRegistry
// may throw RegistryExceptiion.)

#pragma once

#include "stdafx.h"
#include "RegistryAccess.h"
#include "CommandLineParser.h"
#include "MiscSettings.h"
#include "AppConnection.h"
#include "Matcher.h"

using std::wstring;

class Configuration
{
public:
	// Constructors and destructor
	Configuration();
	Configuration(const Configuration& other);
	Configuration& operator=(const Configuration& other);
	~Configuration();

	bool operator==(const Configuration& other) const;
	bool operator!=(const Configuration& other) const;

	void loadFromCommandLine(const wstring& commandLine);
	inline static const wchar_t* getAllowedKeys() { return L"HIVRP"; }

	void loadFromRegistry(LPCTSTR keyName);
	void saveToRegistry(LPCTSTR keyName);

	// Window matching
	bool windowMatch(HWND hwnd) const;
	bool matchingWindowExists() const;

	// Wrappers for other objects
	MiscSettings& settings = m_settings;
	Matcher& filter = m_filter;
	const AppConnection& connection = m_ac;

	inline const bool canRun() const {
		return connection.isConnectionAlive() || filter.isValid();
	}

	// Variables that are not saved between sessions.
	bool isEnabled; // similar to canRun, but may be set from outside.
	bool isFirstSession;

private:
	// Window matching, internal stuff.
	static BOOL CALLBACK matchingWindowExistsEnumProc(HWND hwnd, LPARAM lParam);
	struct MatchingWindowsExistEnumProcArguments {
		const Configuration* pThis;
		bool* pSuccess;
	};

	MiscSettings m_settings;
	Matcher m_filter;
	AppConnection m_ac;
};
