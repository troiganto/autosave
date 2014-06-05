// OleUtils.h : Contains functions to easily get and manipulate
// various OLE objects representing files.
// Also includes a few utility functions surrounding GetModuleFileName
// and PathMatchSpec.
// Functions may throw OleException on failure.

#pragma once

#include "stdafx.h"
#include <wingdi.h>
#include <ShObjIdl.h>
#include <ShlObj.h>
#include "AutoSaveException.h"
#include "IconBitmap.h"

using std::wstring;
using std::vector;

namespace OleUtils
{
	wstring getSelfPath();
	bool isSelf(const wstring& executablePath);

	bool isShortcutFile(const wstring& file);
	bool isExecutable(const wstring& file);

	wstring getFileDisplayName(const wstring& filePath);
	IShellItem& getFileShellItem(const wstring& filePath);
	IDataObject& getFileDataObject(const wstring& filePath);
	IDataObject& getFileDataObjectWithIcon(const wstring& filePath);

	// Fails silently by returning an empty vector.
	vector<wstring> UnpackFileDataObject(IDataObject& data);
}



class OleException : public AutoSaveException
{
public:
	OleException() : AutoSaveException() {}
	OleException(DWORD errorCode) : AutoSaveException(errorCode) {}
	virtual ~OleException() {}
	virtual inline LPCWSTR exceptionType() const { return L"OleException"; }

};
