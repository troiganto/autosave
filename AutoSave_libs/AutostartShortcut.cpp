#include "stdafx.h"
#include "AutostartShortcut.h"



void AutostartShortcut::create()
{
	wstring selfPath = OleUtils::getSelfPath();

	TCHAR selfWorkingDir[MAX_PATH];
	StringCchCopy(selfWorkingDir, MAX_PATH, selfPath.data());
	PathRemoveFileSpec(selfWorkingDir);

	Shortcut sc;
	IShellLink& link = sc.link();
	link.SetPath(selfPath.data());
	link.SetWorkingDirectory(selfWorkingDir);
	link.SetDescription(L"Auto-start shortcut for " APP_NAME);
	sc.save(locate(), TRUE);
}



void AutostartShortcut::remove()
{
	if (DeleteFile(locate().data()) == FALSE)
	{
		// To us, deleting a non-existing file is not a failure.
		DWORD error = GetLastError();
		if (error != ERROR_FILE_NOT_FOUND)
			throw OleException(error);
	}
}



// Strange name to avoid confusion with Shortcut::getPath.
wstring AutostartShortcut::locate()
{
	// First get directory for auto-starts.
	LPTSTR startupDir;
	throwOnFailure<OleException>(
		SHGetKnownFolderPath(FOLDERID_Startup, 0, NULL, &startupDir));

	// Then append generic shortcut file name.
	wstring shortcutPath = startupDir;
	shortcutPath.append(L"\\" APP_NAME L".lnk");
	CoTaskMemFree(startupDir);
	return shortcutPath;
}


