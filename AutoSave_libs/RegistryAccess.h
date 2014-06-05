// RegistryAccess.h : A class that grants easy access to subkeys of the
// HKEY_CURRENT_USER\Software\ key.
// The constructor never throws. All other functions may throw
// RegistryException on failure or missing registry value.

#pragma once

#include "stdafx.h"
#include "AutoSaveException.h"

using std::wstring;
using std::vector;
using std::pair;

class RegistryAccess
{
public:

	RegistryAccess() : m_targetKey(0), m_hasCreatedKey(false), m_keyName(L"") {}
	~RegistryAccess() { close(); }

	void access(const wstring& keyName);
	void close();
	static bool keyExists(const wstring& keyName);

	inline bool isOpen() const { return m_targetKey != 0; }
	inline bool hasKeyBeenCreated() const { return m_hasCreatedKey; }

	int readInt(LPCTSTR valueName) const;
	void writeInt(LPCTSTR valueName, int valueData);

	wstring readString(LPCTSTR valueName) const;
	void writeString(LPCTSTR valueName, const wstring& valueData);

	// Fails silently if the key doesn't exist.
	static wstring readKeyDefaultString(HKEY hBaseKey, LPCTSTR keyPath);

	// Append* fails silently if the key doesn't exist.
	void AppendToMultiString(LPCTSTR valueName, const wstring& value);
	vector<wstring> readMultiString(const LPCTSTR valueName) const;
	void writeMultiString(LPCTSTR valueName, const vector<wstring>& strings);

	// Boring functions for extensibility.
	static inline HKEY getRootKey() { return HKEY_CURRENT_USER; }
	static inline wstring getKeyPath(const wstring& k) { return L"SOFTWARE\\" + k; }
	inline wstring getKeyPath() const { return getKeyPath(m_keyName); }
	inline LPCTSTR getParentKeyPath() const { return L"SOFTWARE"; }
	inline wstring getKeyName() const { return m_keyName; }

	// Automatically calls the close member function.
	void purge();

protected:
	static LPBYTE read(HKEY hParentKey, LPCTSTR keyName, LPCTSTR valueName,
		DWORD valueRestriction);

private:
	static vector<wstring> multiStringToVector(LPCTSTR pString);
	static wstring vectorToMultiString(const vector<wstring>& strings);

	HKEY m_targetKey;
	bool m_hasCreatedKey;
	wstring m_keyName;

};



class RegistryException: public AutoSaveException
{
public:
	RegistryException() : AutoSaveException() {}
	RegistryException(LONG errorCode) : AutoSaveException(errorCode) {}
	virtual ~RegistryException() {}
	virtual inline LPCWSTR exceptionType() const { return L"RegistryException"; }

};
