#include "stdafx.h"
#include "MiscSettings.h"


// Constructor
MiscSettings::MiscSettings()
	: m_hotkey(MAKEWORD(0x53, HOTKEYF_CONTROL)), // ctrl+s
	  m_interval(5 * 60), // five minutes
	  m_verbosity(ALERT_START) // show start, but no 5-second alerts
{
}



// Destructor
MiscSettings::~MiscSettings()
{
}



bool MiscSettings::operator==(const MiscSettings& other) const
{
	return m_hotkey == other.m_hotkey &&
		m_interval == other.m_interval &&
		m_verbosity == other.m_verbosity;
}

bool MiscSettings::operator!=(const MiscSettings& other) const
{
	return !(*this == other);
}



void MiscSettings::loadFromCommandLine(const CommandLineParser& cli)
{
	if (cli.kwArgsContain(L'H'))
		setHotkey(LOWORD(cli.getIntKwArg(L'H')));

	if (cli.kwArgsContain(L'I'))
		setInterval(cli.getIntKwArg(L'I'));

	if (cli.kwArgsContain(L'V'))
		setVerbosity(cli.getIntKwArg(L'V'));
}



// Result of this function is either an empty string,
// or it ends with a trailing space character.
wstring MiscSettings::toCommandLine(int attributesMask) const
{
	if (attributesMask == ATT_NONE)
		return L"";

	const int bufferSize = 42;
	TCHAR buffer[bufferSize];
	wstring formatString;

	if (attributesMask & ATT_INTERVAL)
		formatString.append(L"/I %1!u! ");
	if (attributesMask & ATT_HOTKEY)
		formatString.append(L"/H 0x%2!04x! ");
	if (attributesMask & ATT_VERBOSITY)
		formatString.append(L"/V %3!u! ");
	INT_PTR args[] = { getInterval(), getHotkey(), getVerbosity() };

	throwIfZero<AutoSaveException>(
		FormatMessage(
			FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_STRING,
			formatString.data(), 0, LANG_NEUTRAL,
			buffer, bufferSize, (va_list*)args)
		);
	return buffer;
}



