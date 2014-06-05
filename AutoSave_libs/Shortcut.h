// Shortcut.h : class for all handling of shortcut files.
// Mostly wraps around the CLSID_ShellLink object.
// Most member functions may throw OleException on failure.
// The constructor may throw.
// Beware: This class expects OleInitialize to have been called already!

#pragma once

#include "stdafx.h"
#include "OleUtils.h"
#include <ShObjIdl.h>
#include <ShlObj.h>
#include <ShlGuid.h>

using std::wstring;

class Shortcut
{
public:
	Shortcut();
	~Shortcut();
	const bool& successfullyCreated = m_successfullyCreated;

	// Don't use these when successfullyCreated is false!
	inline IShellLink& link() const { return *m_pLink; }
	inline IPersistFile& file() const { return *m_pFile; }

	void load(const wstring& filePath, DWORD mode);
	void save(const wstring& filePath, BOOL remember);

	wstring getPath() const;
	wstring getRawPath() const;
	wstring getArguments() const;
	wstring getWorkingDirectory() const;
	wstring getDescription() const; // Fails silently.
	wstring getIconLocation(int* pIconIndex) const;

protected:
	bool m_successfullyCreated;
	IShellLink* m_pLink;
	IPersistFile* m_pFile;
};