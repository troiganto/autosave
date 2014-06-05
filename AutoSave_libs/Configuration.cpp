#include "stdafx.h"
#include "Configuration.h"


// Constructor
Configuration::Configuration()
	: m_settings(),
	  m_filter(L"SAI", L"\\.psd|gimp|sai|paint", false),
	  m_ac(),
	  isEnabled(true),
	  isFirstSession(false)
{
}



// Destructor
Configuration::~Configuration()
{
	m_ac.disconnect(); // Unnecessary, but doesn't hurt.
}



// Copy constructor
Configuration::Configuration(const Configuration &other)
	: m_settings(other.m_settings),
	  m_filter(other.m_filter),
	  m_ac(other.m_ac),
	  isEnabled(other.isEnabled),
	  isFirstSession(other.isFirstSession)
{
}



// Copy assignment constructor
Configuration& Configuration::operator=(const Configuration &other)
{
	if (this != &other)
	{
		m_settings = other.m_settings;
		m_filter = other.m_filter;
		m_ac = other.m_ac;
		isEnabled = other.isEnabled;
		isFirstSession = other.isFirstSession;
	}
	return *this;
}



bool Configuration::operator==(const Configuration& other) const
{
	// We call getFilter because for equality, we're only
	// interested in the one filter that matters.
	return m_settings == other.m_settings &&
		m_filter == other.m_filter &&
		m_ac.getProcessId() == other.m_ac.getProcessId();
}

bool Configuration::operator!=(const Configuration& other) const
{
	return !(*this == other);
}



void Configuration::loadFromCommandLine(const wstring& commandLine)
{
	CommandLineParser cli;
	cli.setAllowedKeys(getAllowedKeys());
	cli.parse(commandLine);
	if (!cli.gotArgs())
		return;

	m_settings.loadFromCommandLine(cli);

	if (cli.kwArgsContain(L'R')) {
		filter.setFilter(cli.getStringKwArg(L'R'), true);
	}
	else if (cli.kwArgsContain(L'F')) {
		filter.setFilter(cli.getStringKwArg(L'F'), false);
	}
	else if (!cli.getLArgs().empty()) {
		filter.setFilter(L"", false);
		m_ac.connect(cli.getLArgs());
	}
}



void Configuration::loadFromRegistry(LPCTSTR keyName)
{
	isFirstSession = !RegistryAccess::keyExists(keyName);
	if (isFirstSession)
		return;

	RegistryAccess ra;
	ra.access(keyName);
	m_settings.setHotkey(LOWORD(ra.readInt(L"hotkey")));
	m_settings.setInterval(ra.readInt(L"interval"));
	m_settings.setVerbosity(ra.readInt(L"verbosity"));
	filter.setPhrase(ra.readString(L"filterPhrase"));
	filter.setRegex(ra.readString(L"filterRegex"));
	filter.useRegex(ra.readInt(L"isFilterByRegex") != 0);
}



void Configuration::saveToRegistry(LPCTSTR keyName)
{
	RegistryAccess ra;
	ra.access(keyName);
	ra.writeInt(L"hotkey", m_settings.getHotkey());
	ra.writeInt(L"interval", m_settings.getInterval());
	ra.writeInt(L"verbosity", (UINT)m_settings.getVerbosity());
	ra.writeString(L"filterPhrase", filter.getPhrase());
	ra.writeString(L"filterRegex", filter.getRegex());
	ra.writeInt(L"isFilterByRegex", (UINT)filter.isRegex());
}



bool Configuration::windowMatch(HWND hwnd) const
{
	if (!canRun())
	{
		return false;
	}
	else if (connection.isConnected())
	{
		DWORD windowProcessId;
		GetWindowThreadProcessId(hwnd, &windowProcessId);
		return windowProcessId == connection.getProcessId();
	}
	else {
		wstring caption = Matcher::getWindowText(hwnd);
		return caption.empty() ? false : filter.match(caption);
	}
}



bool Configuration::matchingWindowExists() const
{
	if (canRun())
	{
		bool success = false;
		MatchingWindowsExistEnumProcArguments lParam = { this, &success };
		EnumWindows(matchingWindowExistsEnumProc, (LPARAM)&lParam);
		return success;
	}
	else {
		return false;
	}
}

BOOL CALLBACK Configuration::matchingWindowExistsEnumProc(
	HWND hwnd, LPARAM lParam)
{
	auto args = (MatchingWindowsExistEnumProcArguments*) lParam;
	if (args->pThis->windowMatch(hwnd))
	{
		*args->pSuccess = true;
		return FALSE;
	}
	else {
		return TRUE;
	}
}


