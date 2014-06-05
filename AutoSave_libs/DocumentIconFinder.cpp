#include "stdafx.h"
#include "DocumentIconFinder.h"


DocumentIconFinder::DocumentIconFinder(const wstring& documentPath)
	: iconFile(m_iconFile), iconIndex(m_iconIndex)
{
	findIconInRegistry(documentPath);
	if (m_iconFile == L"%1")
	{
		// Special case: Some file types (e.g. .ico) refer to themselves as icon.
		m_iconFile = documentPath;
	}
	else if (m_iconFile.empty())
	{
		// Use default icon if we couldn't find a matching one.
		useDefaultIcon();
	}
}



DocumentIconFinder::~DocumentIconFinder()
{
}



void DocumentIconFinder::useDefaultIcon()
{
	m_iconFile = OleUtils::getSelfPath();
	m_iconIndex = -IDI_CONNECTEDFILE;
}



void DocumentIconFinder::findIconInRegistry(const wstring& documentPath)
{
	m_iconFile.clear();
	m_iconIndex = 0;

	wstring fileType = PathFindExtension(documentPath.data());
	if (fileType.empty())
		return;

	// First attempt: Get .ext\DefaultIcon's default value.
	wstring iconEntry = RegistryAccess::readKeyDefaultString(
		HKEY_CLASSES_ROOT, (fileType + L"\\DefaultIcon").data());
	if (iconEntry.empty())
	{
		// Second attempt: Get .ext's default value and look it up.
		wstring fileTypeHandler = RegistryAccess::readKeyDefaultString(
			HKEY_CLASSES_ROOT, fileType.data());
		iconEntry = RegistryAccess::readKeyDefaultString(
			HKEY_CLASSES_ROOT, (fileTypeHandler + L"\\DefaultIcon").data());
	}

	if (!iconEntry.empty())
		useIconFromFile(iconEntry);
}



void DocumentIconFinder::useIconFromFile(const wstring& iconFile)
{
	TCHAR buffer[MAX_PATH];
	throwOnFailure<OleException>(
		StringCchCopy(buffer, MAX_PATH, iconFile.data()));
	m_iconIndex = PathParseIconLocation(buffer);
	m_iconFile = buffer;
}



