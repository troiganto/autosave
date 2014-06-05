// DocumentIconFinder.h: A small class whose purpose it is to locate
// the icon associated with a certain file type.
// May throw AutoSaveException and descendants on failure.

#pragma once

#include "stdafx.h"
#include "OleUtils.h"
#include "RegistryAccess.h"
#include "..\AutoSave\Resource.h"

using std::wstring;

class DocumentIconFinder
{
public:
	DocumentIconFinder(const wstring& documentPath);
	~DocumentIconFinder();

	const wstring& iconFile;
	const int& iconIndex;

private:
	void findIconInRegistry(const wstring& documentPath);
	void useDefaultIcon();
	void useIconFromFile(const wstring& iconFile);

	static inline bool isIconFile(const wstring& filePath) {
		return PathMatchSpec(filePath.data(), L"*.ico") != FALSE;
	}

	wstring m_iconFile;
	int m_iconIndex;
};

