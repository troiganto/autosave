#include "stdafx.h"
#include "CommandLineParser.h"


CommandLineParser::CommandLineParser()
{
}



CommandLineParser::~CommandLineParser()
{
}



void CommandLineParser::parse(const wstring& commandLine)
{
	clear();
	vector<wstring> args = split(commandLine);

	wchar_t currentKey = 0;
	bool inKwArgsSection = true;
	for (const wstring& arg : args)
	{
		// Second part of the AND is indicating that the LArg section has begun.
		if (inKwArgsSection && (isKeyArgument(arg) || (currentKey != 0)))
		{
			if (currentKey == 0)
			{
				// First step of kwarg consumption.
				checkValidKey(arg);
				currentKey = arg.at(1);
			}
			else {
				// Second step of kwarg consumption.
				m_kwargs.emplace(currentKey, arg);
				currentKey = 0;
			}
		}
		else {
			// Only step of larg consumption.
			inKwArgsSection = false;
			m_largs.push_back(arg);
		}
	}
	checkMissingValue(currentKey);
}



vector<wstring> CommandLineParser::split(const wstring& commandLine)
{
	vector<wstring> result;
	if (commandLine.empty())
		return result;

	int nArgs = 0;
	LPWSTR* argList = CommandLineToArgvW(commandLine.data(), &nArgs);
	if (argList == NULL)
		return result;

	for (int i = 0; i < nArgs; ++i)
	{
		result.push_back(argList[i]);
	}
	LocalFree(argList);
	return result;
}



bool CommandLineParser::isKeyArgument(const wstring& keyArg)
{
	return keyArg.length() > 1 && keyArg[0] == L'/';
}



bool CommandLineParser::isValidKeyArgument(const wstring& keyArg) const
{
	return isKeyArgument(keyArg) && keyArg.length() == 2 &&
		(m_allowedKeys.find(keyArg[1]) != wstring::npos);
}



void CommandLineParser::checkValidKey(const wstring& arg)
{
	if (!isValidKeyArgument(arg)) {
		clear();
		throw CLIException(E_INVALIDARG);
	}
}



void CommandLineParser::checkMissingValue(wchar_t currentKey)
{
	if (currentKey != 0) {
		clear();
		throw CLIException(E_INVALIDARG);
	}
}



wstring CommandLineParser::escapeArgument(const wstring& arg)
{
	wstring result;
	result.reserve(arg.size() + 2);
	bool spaceFound = false;
	int backslashCount = 0;
	for (wchar_t letter : arg)
	{
		if (letter == L'\\')
		{
			// Remember the amount of backslashes to escape later.
			++backslashCount;
		}
		else if (letter == L'"') {
			// Escape backslashes AND the quotation mark.
			result.append(backslashCount + 1, L'\\');
		}
		else {
			// Quote spaces later.
			if (letter == L' ')
				spaceFound = true;
			// No quotation mark, no escaping.
			backslashCount = 0;
		}
		result.push_back(letter);
	}
	// Quote everything if there is at least one space.
	if (spaceFound)
	{
		result.insert(result.begin(), L'"');
		result.push_back(L'"');
	}
	return result;
}



int CommandLineParser::getIntKwArg(const wchar_t keyName) const
{
	wstring argAsString = m_kwargs.at(keyName);
	size_t idx;
	int argAsInt = stoi(argAsString, &idx, 0);
	if (idx != argAsString.size())
		throw std::invalid_argument("not an integer argument");
	return argAsInt;
}



const wstring CommandLineParser::getStringKwArg(const wchar_t keyName) const
{
	return m_kwargs.at(keyName);
}



bool CommandLineParser::kwArgsContain(const wchar_t keyName) const
{
	return m_kwargs.count(keyName) == 1;
}



wstring CommandLineParser::joinArguments(const vector<wstring>& args)
{
	return joinArguments(args.cbegin(), args.cend());
}



wstring CommandLineParser::joinArguments(
	vector<wstring>::const_iterator argsBegin,
	vector<wstring>::const_iterator argsEnd)
{
	wstring result;
	for (auto pItem = argsBegin; pItem < argsEnd; ++pItem)
	{
		result.append(escapeArgument(*pItem));
		result.push_back(L' ');
	}
	// Remove superfluous space if result is non-empty.
	if (!result.empty())
		result.pop_back();
	return result;
}




