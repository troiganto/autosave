// TemporaryShortcut.h: Creates, handles, and deletes a shortcut file
// in the temp folder and fills it with the properties of a connected shortcut.
// Throws OleException in most cases of failure.
// May throw RegistryException or AutoSaveException if DocumentIconFinder fails.

#pragma once

#include "ConnectedShortcut.h"
#include "OleUtils.h"
#include "MiscSettings.h"
#include "DocumentIconFinder.h"

using std::wstring;

class TemporaryShortcutFile : public ConnectedShortcut
{
public:
	TemporaryShortcutFile();
	~TemporaryShortcutFile();

	void updateFile(const wstring& targetFilePath);
	void updateSettings(const MiscSettings& customSettings, int settingsMask);

	inline bool isNull() const { return m_currentFile == L""; }
	inline wstring getFile() const { return m_currentFile; }

	// The directory where the file is saved -- defaults to %TEMP%
	inline const wstring& getSaveDirectory() const { return m_saveDirectory; }
	void setSaveDirectory(const wstring& newDir) {
		m_saveDirectory = newDir;
		if (m_saveDirectory.back() != L'\\')
			m_saveDirectory.push_back(L'\\');
	}

protected:
	void deleteFile();
	void updateTargetFile(const wstring& newFileName);

private:
	void fillFromFile(const wstring& targetPath);
	void fillFromShortcut(const wstring& shortcutPath);

	void setIconLocationFromShortcut(const Shortcut& sc);
	wstring combineArgs() const;

	static wstring getParentDir(const wstring& path);
	static wstring getShortcutWorkingDir(const Shortcut& sc);
	
	wstring m_saveDirectory;
	wstring m_currentFile;
	
	wstring m_configArgs;
	
};





// This is a workaround to a bug in IShellLink.
// In certain shortcuts, GetPath redirects subdirectories of Program Files
// to Program Files (x86), even if it doesn't make sense.
// Wow64DisableWow64FsRedirection does not work for this.
// Cf. http://social.msdn.microsoft.com/Forums/en-US/6f2e7920-50a9-459d-bfdd-316e459e87c0
// or http://social.msdn.microsoft.com/Forums/en-US/41a714d2-d569-4d8e-9259-8f4645e426d4
// for more information.
// The x64 version of this class just doesn't do anything at all.
#ifdef _WIN64
class TemporaryRepairedShortcut
{
public:
	inline const wstring& repair(const wstring& file) { return file; }
	const bool successfullyCreated = true;
};
#else
// The non-dummy class starts here.
class TemporaryRepairedShortcut : private Shortcut
{
public:
	TemporaryRepairedShortcut();
	~TemporaryRepairedShortcut();

	const bool& successfullyCreated = m_successfullyCreated;
	const wstring repair(const wstring& filePath);

private:
	IShellLinkDataList* m_pList;
	wstring m_tempDir;
	wstring m_fileName;

	void makeTempDir();
	void repairShortcut();
	void clear();
};
#endif