#include "stdafx.h"
#include "Matcher.h"


Matcher::Matcher()
	: m_phrase(),
	  m_regex(),
	  m_loweredPhrase(),
	  m_regexObject(),
	  m_isFilterByRegex(false),
	  m_isRegexBad(false)
{
}



Matcher::Matcher(const wstring& filter, bool isRegex) : Matcher()
{
	setFilter(filter, isRegex);
}



Matcher::Matcher(const wstring& filterPhrase,
	const wstring& filterRegex, bool isRegex)
{
	setPhrase(filterPhrase);
	setRegex(filterRegex);
	useRegex(isRegex);
}



Matcher::~Matcher()
{
}



bool Matcher::operator==(const Matcher& other) const
{
	return isRegex() == other.isRegex() && getFilter() == other.getFilter();
}

bool Matcher::operator!=(const Matcher& other) const
{
	return !(*this == other);
}



void Matcher::setPhrase(const wstring& phrase)
{
	m_phrase.assign(phrase);
	m_loweredPhrase = toLower(m_phrase);
}



void Matcher::setRegex(const wstring& regex)
{
	m_regex.assign(regex);
	try{
		m_regexObject.assign(m_regex,
			(std::regex_constants::syntax_option_type) syntaxFlags);
		m_isRegexBad = false;
	}
	catch (std::regex_error&) {
		m_isRegexBad = true;
	}
}



void Matcher::setFilter(const wstring& filter, bool isRegex)
{
	m_isFilterByRegex = isRegex;
	isRegex ? setRegex(filter) : setPhrase(filter);
}



void Matcher::setFilter(const Matcher& other)
{
	setFilter(other.getFilter(), other.isRegex());
}



bool Matcher::match(const wstring& text) const
{
	if (!isValid())
		return false;

	if (m_isFilterByRegex)
	{
		return regex_search(text, m_regexObject,
			(std::regex_constants::match_flag_type) matchFlags);
	}
	else {
		return toLower(text).find(m_loweredPhrase) != wstring::npos;
	}
}



wstring Matcher::getWindowText(HWND hwnd)
{
	const int textLength = GetWindowTextLength(hwnd);
	if (textLength == 0)
		return L"";

	std::auto_ptr<TCHAR> buffer(new TCHAR[textLength + 1]);
	GetWindowText(hwnd, buffer.get(), textLength + 1);
	return buffer.get();
}



wstring Matcher::toLower(const wstring& s)
{
	wstring result;
	result.reserve(s.size() + 1);
	for (wchar_t c : s)
	{
		result.push_back(towlower(c));
	}
	return result;
}

