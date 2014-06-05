#include "stdafx.h"
#include "Shortcut.h"


Shortcut::Shortcut() : m_successfullyCreated(false)
{
	throwOnFailure<OleException>(
		CoCreateInstance(
		CLSID_ShellLink, NULL, CLSCTX_ALL, IID_IShellLink, (LPVOID*)&m_pLink));
	throwOnFailure<OleException>(
		m_pLink->QueryInterface<IPersistFile>(&m_pFile));
	m_successfullyCreated = true;
}


Shortcut::~Shortcut()
{
	if (m_pLink != NULL)
		m_pLink->Release();
	if (m_pFile != NULL)
		m_pFile->Release();
	m_pLink = NULL;
	m_pFile = NULL;
}



void Shortcut::load(const wstring& filePath, DWORD mode)
{
	throwOnFailure<OleException>(
		m_pFile->Load(filePath.data(), mode));
}



void Shortcut::save(const wstring& filePath, BOOL remember)
{
	throwOnFailure<OleException>(
		m_pFile->Save(filePath.data(), remember));
}



wstring Shortcut::getPath() const
{
	TCHAR buffer[MAX_PATH];
	throwOnFailure<OleException>(
		m_pLink->GetPath(buffer, MAX_PATH, NULL, 0));
	return buffer;
}



wstring Shortcut::getRawPath() const
{
	TCHAR buffer[MAX_PATH];
	throwOnFailure<OleException>(
		m_pLink->GetPath(buffer, MAX_PATH, NULL, SLGP_RAWPATH));
	return buffer;
}



wstring Shortcut::getArguments() const
{
	// There is no reason to use this length specifically,
	// but we hope that this is sufficiently large.
	TCHAR buffer[INFOTIPSIZE];
	throwOnFailure<OleException>(
		m_pLink->GetArguments(buffer, INFOTIPSIZE));
	return buffer;
}



wstring Shortcut::getWorkingDirectory() const
{
	TCHAR buffer[MAX_PATH];
	throwOnFailure<OleException>(
		m_pLink->GetWorkingDirectory(buffer, MAX_PATH));
	return buffer;
}



wstring Shortcut::getDescription() const
{
	TCHAR buffer[INFOTIPSIZE];
	m_pLink->GetDescription(buffer, INFOTIPSIZE);
	return buffer;
}



wstring Shortcut::getIconLocation(int* pIconIndex) const
{
	TCHAR buffer[MAX_PATH];
	throwOnFailure<OleException>(
		m_pLink->GetIconLocation(buffer, MAX_PATH, pIconIndex));
	return buffer;
}

