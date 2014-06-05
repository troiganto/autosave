// Matcher.h : Wraps the switching between regular string
// matching (phrase) and using regular expressions (regex).
// Doesn't throw exceptions.

#pragma once

#include "stdafx.h"

using std::wstring;
using std::wregex;

class Matcher
{
public:
	Matcher();
	Matcher(const wstring& filter, bool isRegex = false);
	Matcher(const wstring& filterPhrase, const wstring& filterRegex,
		bool isRegex = false);
	~Matcher();

	bool operator==(const Matcher& other) const;
	bool operator!=(const Matcher& other) const;

	// Getters and setters.

	inline wstring getPhrase() const { return m_phrase; }
	void setPhrase(const wstring& phrase);

	inline wstring getRegex() const { return m_regex; }
	void setRegex(const wstring& regex);

	inline wstring getFilter() const {
		return m_isFilterByRegex ? m_regex : m_phrase;
	}
	void setFilter(const wstring& filter, bool isRegex);
	void setFilter(const Matcher& other); // Copies less than the copy constructor.

	inline bool isRegex() const { return m_isFilterByRegex; }
	inline void useRegex(bool isRegex) { m_isFilterByRegex = isRegex; }

	// Validity checks.
	inline bool isEmpty() const { return getFilter() == L""; }
	inline bool isRegexBad() const { return m_isRegexBad; }
	inline bool isValid() const {
		return !isEmpty() && (isRegex() ? !isRegexBad() : true);
	}

	// The actually important function.
	bool match(const wstring& text) const;

	// Utility function: std::wstring wrapper around GetWindowText.
	static wstring getWindowText(HWND hwnd);

private:
	static wstring toLower(const wstring& s);

	wstring m_phrase;
	wstring m_regex;

	wstring m_loweredPhrase;
	wregex m_regexObject;

	bool m_isFilterByRegex;
	bool m_isRegexBad;

	// Sadly, this is the only way this works
	enum {
		syntaxFlags = std::regex_constants::icase +
		std::regex_constants::nosubs +
		std::regex_constants::collate +
		std::regex_constants::ECMAScript
	};
	enum {
		matchFlags = std::regex_constants::match_any +
		std::regex_constants::match_not_null
	};

};
