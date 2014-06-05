// AutoSaveException.h :
// Base class for exceptions that wrap around Windows' error codes.
// Handles both ERROR_* and HRESULT codes.
// Derives from std::runtime_error.
// Includes two utility function templates, throwOnFailure and throwIfZero.

#pragma once

#include "stdafx.h"

using std::wstring;

class AutoSaveException : public std::runtime_error
{
public:
	AutoSaveException(DWORD errorCode);
	AutoSaveException() : AutoSaveException(GetLastError()) {}
	~AutoSaveException();
	
	// Getting numeric error codes.
	inline HRESULT hResult() const { return m_hResult; }
	inline long errorCode() const { return HRESULT_CODE(m_hResult); }

	// Getting string messages.
	virtual LPCSTR what();
	virtual LPCWSTR wcwhat();

	// Utility member functions for UI purposes.
	virtual inline LPCWSTR exceptionType() const { return L"AutoSaveException"; }
	virtual void showMessageBox(HWND hwnd, wstring text);
	static void showMessageBox(HWND hwnd, wstring text, std::exception& exc);

private:
	static wstring ansiToWide(LPCSTR ansi);

	HRESULT m_hResult;
	char* m_ansiBuffer;
	wchar_t* m_wideBuffer;
};



// Utility function for easier throwing.
// Usage: throwOnFailure<ExceptionClass>(
//            StringCchCopy(buffer, 260, const_string.data()));
template<typename E>
void throwOnFailure(HRESULT hres)
{
	if (FAILED(hres))
		throw E(hres);
}


// Utility function for easier throwing.
// Usage: my_id = throwIfZero<ExceptionClass>(
//            getThatId());
template<typename E, typename T>
T throwIfZero(T errorCode)
{
	if (errorCode == (T)0)
		throw E();
	return errorCode;
}

