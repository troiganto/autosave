// CommandLineParser.h : Object that parses command lines of the style
// LR"(/A 10 /B "key-word argument" /C 42 "list argument" arg arg)".
// Functions may throw CLIException.
// The function getIntKwarg may additionally throw if std::stoi throws.

#pragma once

#include "stdafx.h"
#include "AutoSaveException.h"

using std::wstring;
using std::vector;
using std::unordered_map;

class CommandLineParser
{
public:
	CommandLineParser();
	~CommandLineParser();

	// Changing the object state.
	inline void clear() { m_kwargs.clear(); m_largs.clear(); }
	void parse(const wstring& commandLine); //throws CLIException

	// Modifiers for the parsing process.
	inline wstring getAllowedKeys() const { return m_allowedKeys; }
	inline void setAllowedKeys(const wstring& allowedKeys) {
		m_allowedKeys = allowedKeys;
	}
	inline void allowAllKeys() {
		m_allowedKeys = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	}

	// Content queries.
	
	inline bool gotKwArgs() const { return !m_kwargs.empty(); }
	inline bool gotLArgs() const { return !m_largs.empty(); }
	inline bool gotArgs() const { return gotKwArgs() || gotLArgs(); }

	bool kwArgsContain(const wchar_t keyName) const;
	int getIntKwArg(const wchar_t keyName) const;
	const wstring getStringKwArg(const wchar_t keyName) const;

	inline const vector<wstring>& getLArgs() const { return m_largs; }
	inline wstring getLArgsString() const { return joinArguments(m_largs); }

	// Static utility functions.
	static wstring escapeArgument(const wstring& arg);
	static wstring joinArguments(const vector<wstring>& args);
	static wstring joinArguments(vector<wstring>::const_iterator argsBegin,
		vector<wstring>::const_iterator argsEnd);
	static vector<wstring> split(const wstring& commandLine);

private:
	static bool isKeyArgument(const wstring& key);
	bool isValidKeyArgument(const wstring& key) const;

	void checkValidKey(const wstring& arg);
	void checkMissingValue(wchar_t currentKey);

	unordered_map<wchar_t, wstring> m_kwargs;
	vector<wstring> m_largs;
	wstring m_allowedKeys;
};



class CLIException : public AutoSaveException
{
public:
	CLIException() : AutoSaveException() {}
	CLIException(DWORD errorCode) : AutoSaveException(errorCode) {}
	virtual ~CLIException() {}
	virtual inline LPCWSTR exceptionType() const { return L"CLIException"; }
};
