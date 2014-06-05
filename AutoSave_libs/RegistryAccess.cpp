#include "stdafx.h"
#include "RegistryAccess.h"


bool RegistryAccess::keyExists(const wstring& keyName)
{
	HKEY key;
	LONG result = RegOpenKeyEx(
		getRootKey(), getKeyPath(keyName).data(), 0, KEY_READ, &key);

	if (result == ERROR_SUCCESS)
	{
		RegCloseKey(key);
		return true;
	}
	else {
		return false;
	}
}



void RegistryAccess::access(const wstring& keyName)
{
	if (keyName.empty())
		throw RegistryException(ERROR_INVALID_NAME);
	close();

	m_keyName = keyName;
	m_hasCreatedKey = false;
	DWORD disposition;

	throwOnFailure<RegistryException>(
		RegCreateKeyEx(
		HKEY_CURRENT_USER, getKeyPath().data(), 0, 0,
		REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE | DELETE, NULL,
		&m_targetKey, &disposition));

	m_hasCreatedKey = (disposition == REG_CREATED_NEW_KEY);
}

void RegistryAccess::close()
{
	if (isOpen())
	{
		RegCloseKey(m_targetKey);
		m_keyName.clear();
		m_targetKey = 0;
		m_hasCreatedKey = false;
	}
}



int RegistryAccess::readInt(LPCTSTR valueName) const
{
	LPBYTE buffer = read(m_targetKey, L"", valueName, RRF_RT_REG_DWORD);
	int result = *(int*) buffer;
	HeapFree(GetProcessHeap(), 0, buffer);
	return result;
}



void RegistryAccess::writeInt(LPCTSTR valueName, int valueData)
{
	throwOnFailure<RegistryException>(
		RegSetValueEx(m_targetKey, valueName, 0, REG_DWORD,
		(LPBYTE)&valueData, sizeof(valueData)));
}



wstring RegistryAccess::readString(LPCTSTR valueName) const
{
	LPBYTE buffer = read(m_targetKey, L"", valueName, RRF_RT_REG_SZ);
	wstring result = (LPCTSTR) buffer;
	HeapFree(GetProcessHeap(), 0, buffer);
	return result;
}



void RegistryAccess::writeString(LPCTSTR valueName, const wstring& valueData)
{
	DWORD byteCount = (1 + (int) valueData.length()) * sizeof(wchar_t);
	throwOnFailure<RegistryException>(
		RegSetValueEx(m_targetKey, valueName, 0, REG_SZ,
		(LPBYTE)valueData.data(), byteCount));
}



vector<wstring> RegistryAccess::readMultiString(LPCTSTR valueName) const
{
	LPBYTE buffer = read(m_targetKey, L"", valueName, RRF_RT_REG_MULTI_SZ);
	vector<wstring> results = multiStringToVector((LPCTSTR) buffer);
	HeapFree(GetProcessHeap(), 0, buffer);
	return results;
}



void RegistryAccess::writeMultiString(
	LPCTSTR valueName, const vector<wstring>& strings)
{
	wstring string = vectorToMultiString(strings);
	DWORD byteCount = (int) string.length() * sizeof(wchar_t);
	throwOnFailure<RegistryException>(
		RegSetValueEx(m_targetKey, valueName, 0, REG_MULTI_SZ,
		(LPBYTE)string.data(), byteCount));
}



void RegistryAccess::AppendToMultiString(LPCTSTR valueName, const wstring& value)
{
	vector<wstring> strings;
	try {
		strings = readMultiString(valueName);
	}
	catch (RegistryException& exc) {
		// If the value doesn't exist, we silently create it.
		if (exc.errorCode() != ERROR_FILE_NOT_FOUND)
			throw;
	}
	strings.push_back(value);
	writeMultiString(valueName, strings);
}



wstring RegistryAccess::readKeyDefaultString(HKEY hBaseKey, LPCTSTR keyPath)
{
	try {
		LPBYTE buffer = read(hBaseKey, keyPath, L"", RRF_RT_REG_SZ);
		wstring result = (LPCTSTR) buffer;
		HeapFree(GetProcessHeap(), 0, buffer);
		return result;
	}
	catch (RegistryException& exc) {
		if (exc.errorCode() != ERROR_FILE_NOT_FOUND)
			throw;
		return L"";
	}
}



void RegistryAccess::purge()
{
	if (!isOpen())
		throw RegistryException(ERROR_FILE_INVALID);

	// Delete contents of the key.
	throwOnFailure<RegistryException>(
		RegDeleteTree(m_targetKey, NULL));

	// Open parent key.
	HKEY parentKey;
	throwOnFailure<RegistryException>(
		RegOpenKeyEx(HKEY_CURRENT_USER, getParentKeyPath(), 0, DELETE, &parentKey));

	// Delete the purged key itself.
	throwOnFailure<RegistryException>(
		RegDeleteKey(parentKey, getKeyName().data()));

	// Close parent key and key itself.
	throwOnFailure<RegistryException>(
		RegCloseKey(parentKey));
	close();
}



LPBYTE RegistryAccess::read(
	HKEY hParentKey, LPCTSTR keyName, LPCTSTR valueName, DWORD valueRestriction)
{
	// Get size of the queried data.
	DWORD bufferSize = 0;
	throwOnFailure<RegistryException>(HRESULT_FROM_WIN32(
		RegGetValue(hParentKey, keyName, valueName, valueRestriction,
			NULL, NULL, &bufferSize)
		));

	// Get data.
	LPBYTE buffer = (LPBYTE) throwIfZero<AutoSaveException>(
		HeapAlloc(GetProcessHeap(), 0, bufferSize));
	LSTATUS regResult = RegGetValue(hParentKey, keyName, valueName, valueRestriction,
		NULL, buffer, &bufferSize);

	// Return or throw error.
	if (regResult == ERROR_SUCCESS)
	{
		return buffer;
	}
	else {
		HeapFree(GetProcessHeap(), 0, buffer);
		throw RegistryException(regResult);
	}
}

vector<wstring> RegistryAccess::multiStringToVector(LPCTSTR pString)
{
	vector<wstring> strings;
	while (*pString != L'\0')
	{
		strings.push_back(pString);
		pString += _tcslen(pString) + 1;
	}
	return strings;
}



wstring RegistryAccess::vectorToMultiString(const vector<wstring>& strings)
{
	wstring result;
	for (const wstring& string : strings)
	{
		result.append(string);
		result.push_back(L'\0');
	}
	return result;
}

