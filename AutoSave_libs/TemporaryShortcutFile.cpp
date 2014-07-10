#include "stdafx.h"
#include "TemporaryShortcutFile.h"

TemporaryShortcutFile::TemporaryShortcutFile()
	: m_currentFile()
{
	const size_t bufferSize = throwIfZero<OleException>(
		GetTempPath(0, NULL));

	std::auto_ptr<TCHAR> buffer(new TCHAR[bufferSize]);
	throwIfZero<OleException>(
		GetTempPath((DWORD)bufferSize, buffer.get()));
	setSaveDirectory(buffer.get());
}



TemporaryShortcutFile::~TemporaryShortcutFile()
{
	try {
		deleteFile();
	}
	catch (...) {
		// Throw under no circumstances.
	}
}


// Always deletes the old file.
// In case of error, returns to the state of pointing to no file at all.
// Unless the file can't be deleted; do nothing then.
void TemporaryShortcutFile::updateFile(const wstring& targetFilePath)
{
	deleteFile();
	wstring wdir;
	updateTargetFile(targetFilePath);
	connect(m_configArgs);
	save(m_currentFile, TRUE);
}



void TemporaryShortcutFile::updateSettings(
	const MiscSettings& customSettings, int settingsMask)
{
	m_configArgs = customSettings.toCommandLine(settingsMask);
	if (!isNull())
	{
		reconnect(m_configArgs);
		save(m_currentFile, TRUE);
	}
}



void TemporaryShortcutFile::deleteFile()
{
	if (isNull() ||
		!PathFileExists(m_currentFile.data()) ||
		DeleteFile(m_currentFile.data()) != FALSE)
	{
		m_currentFile.clear();
	}
	else {
		throw OleException();
	}
}



void TemporaryShortcutFile::updateTargetFile(const wstring& targetFilePath)
{
	if (OleUtils::isShortcutFile(targetFilePath))
	{
		// Workaround for a bug in IShellLink. See comment on the
		// TemporaryRepairedShortcut class for more information.
		// On x64 platforms, trs.repair(a) returns a.
		TemporaryRepairedShortcut trs;
		fillFromShortcut(trs.repair(targetFilePath));
			
	}
	else if (OleUtils::isExecutable(targetFilePath))
	{
		fillFromExecutable(targetFilePath);
	}
	else {
		fillFromDocument(targetFilePath);
	}
	m_currentFile = m_saveDirectory + connectFileName(targetFilePath);
}



void TemporaryShortcutFile::fillFromExecutable(const wstring& targetPath)
{
	// Avoid recursion.
	if (OleUtils::isSelf(targetPath))
		throw OleException(E_INVALIDARG);
	m_pLink->SetPath(targetPath.data());
	m_pLink->SetArguments(L"");
	m_pLink->SetWorkingDirectory(getParentDir(targetPath).data());
	m_pLink->SetDescription(OleUtils::getFileDisplayName(targetPath).data());
	m_pLink->SetIconLocation(targetPath.data(), 0);
}



void TemporaryShortcutFile::fillFromDocument(const wstring& targetPath)
{
	m_pLink->SetPath(targetPath.data());
	m_pLink->SetArguments(L"");
	m_pLink->SetWorkingDirectory(getParentDir(targetPath).data());
	{
		TCHAR buffer[MAX_PATH];
		throwIfZero<OleException>(
			PathCompactPathEx(buffer, targetPath.data(), 65, 0));
		m_pLink->SetDescription(buffer);
	}
	DocumentIconFinder dif(targetPath);
	m_pLink->SetIconLocation(dif.iconFile.data(), dif.iconIndex);
}



void TemporaryShortcutFile::fillFromShortcut(const wstring& shortcutPath)
{
	Shortcut other;
	other.load(shortcutPath, STGM_READ);
	wstring target = other.getRawPath();

	// Avoid recursion and shortcuts to directories.
	if (OleUtils::isSelf(target) || PathIsDirectory(target.data()) != FALSE)
		throw OleException(E_INVALIDARG);

	m_pLink->SetPath(target.data());
	m_pLink->SetArguments(other.getArguments().data());
	m_pLink->SetWorkingDirectory(getShortcutWorkingDir(other).data());
	{
		wstring description = other.getDescription();
		if (description.empty())
			description = OleUtils::getFileDisplayName(shortcutPath);
		m_pLink->SetDescription(description.data());
	}
	setIconLocationFromShortcut(other);
}



// If the given shortcut has an icon, use this.
// If it doesn't, treat it as if it were the file that it is pointing to.
void TemporaryShortcutFile::setIconLocationFromShortcut(const Shortcut& sc)
{
	int iconIndex;
	wstring iconFile = sc.getIconLocation(&iconIndex);
	if (!iconFile.empty())
	{
		m_pLink->SetIconLocation(iconFile.data(), iconIndex);
		return;
	}

	wstring target = sc.getRawPath();
	if (OleUtils::isExecutable(target))
	{
		m_pLink->SetIconLocation(target.data(), 0);
	}
	else {
		DocumentIconFinder dif(target);
		m_pLink->SetIconLocation(dif.iconFile.data(), dif.iconIndex);
	}
}



// Wrapper for the case that the working directory property is empty.
wstring TemporaryShortcutFile::getShortcutWorkingDir(const Shortcut& sc)
{
	wstring result = sc.getWorkingDirectory();
	return result.empty() ? getParentDir(sc.getPath()) : result;
}



wstring TemporaryShortcutFile::getParentDir(const wstring& path)
{
	TCHAR buffer[MAX_PATH];
	throwOnFailure<OleException>(
		StringCchCopy(buffer, MAX_PATH, path.data()));
	PathRemoveFileSpec(buffer);
	return buffer;
}






#ifndef _WIN64
// Cf. the header file for more information on what's this supposed to be.
TemporaryRepairedShortcut::TemporaryRepairedShortcut()
	: m_tempDir(), m_fileName()
{
	m_successfullyCreated = false;
	throwOnFailure<OleException>(
		m_pLink->QueryInterface<IShellLinkDataList>(&m_pList));
	m_successfullyCreated = true;
}

TemporaryRepairedShortcut::~TemporaryRepairedShortcut()
{
	clear();
	if (m_successfullyCreated)
		m_pList->Release();
}



const wstring TemporaryRepairedShortcut::repair(
	const wstring& originalShortcutPath)
{
	clear();
	makeTempDir();
	m_fileName = originalShortcutPath.substr(
		originalShortcutPath.rfind(L'\\') + 1);

	load(originalShortcutPath, STGM_READ);
	repairShortcut();
	save(m_tempDir + m_fileName, TRUE);
	return m_tempDir + m_fileName;
}


void TemporaryRepairedShortcut::makeTempDir()
{
	TCHAR tempParentDir[MAX_PATH];
	DWORD bufferSize = GetTempPath(MAX_PATH, tempParentDir);
	if (bufferSize == 0 || bufferSize > MAX_PATH)
		throw OleException();

	TCHAR tempDirPath[MAX_PATH];
	throwIfZero<OleException>(
		GetTempFileName(tempParentDir, L"BAS", 0, tempDirPath));
	throwIfZero<OleException>(
		DeleteFile(tempDirPath));

	throwIfZero<OleException>(
		CreateDirectory(tempDirPath, NULL));
	m_tempDir = tempDirPath;
	m_tempDir.push_back(L'\\');
}



// The actual repairing: It turns out that Windows does /not/
// redirect x86 applications' calls to IShellLink::GetPath, if you
// remove the flag SLDF_ENABLE_TARGET_METADATA from them.
// Since we can't do this on-the-fly, we need to create a temporary file.
void TemporaryRepairedShortcut::repairShortcut()
{
	DWORD flags;
	throwOnFailure<OleException>(
		m_pList->GetFlags(&flags));

	flags &= ~SLDF_ENABLE_TARGET_METADATA;
	throwOnFailure<OleException>(
		m_pList->SetFlags(flags));
}



void TemporaryRepairedShortcut::clear()
{
	if (!m_fileName.empty())
		DeleteFile((m_tempDir + m_fileName).data());
	if (!m_tempDir.empty())
		RemoveDirectory(m_tempDir.data());
	m_fileName.clear();
	m_tempDir.clear();
}
#endif





