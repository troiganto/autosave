#include "stdafx.h"
#include "OleUtils.h"



namespace {
	wstring makeAbsolutePath(const wstring& path);
	LPITEMIDLIST getFilePidl(const wstring& filePath);
	IDataObject& getFileDataObjectFromPidl(LPCITEMIDLIST pidlFilePath);
	IShellFolder& getDirShellFolder(LPCITEMIDLIST pidlDirPath);
	IShellFolder& getDesktopShellFolder();
	vector<wstring> UnpackHDrop(HDROP dropData);

	// Used by getFileDataObjectWithIcon because that's what it's for.
	class DragHelper {
	public:
		DragHelper();
		~DragHelper();
		IDragSourceHelper2* pdsh;
	};
}



wstring OleUtils::getSelfPath()
{
	wstring selfPath;
	if (selfPath.empty())
	{
		TCHAR buffer[MAX_PATH];
		DWORD size = GetModuleFileName(NULL, buffer, MAX_PATH);
		if (size == 0 || size == MAX_PATH)
			throw OleException();
		selfPath = buffer;
	}
	return selfPath;
}



bool OleUtils::isSelf(const wstring& executablePath)
{
	return _tcsicmp(
		makeAbsolutePath(executablePath).data(), getSelfPath().data()) == 0;
}



bool OleUtils::isShortcutFile(const wstring& file)
{
	return PathMatchSpec(file.data(), L"*.lnk") != FALSE;
}



bool OleUtils::isExecutable(const wstring& file)
{
	if (PathFileExists(file.data()) != FALSE)
	{
		DWORD_PTR exeType = SHGetFileInfo(file.data(), 0, NULL, 0, SHGFI_EXETYPE);
		return exeType != 0;
	}
	else {
		return PathMatchSpec(file.data(), L"*.exe") != FALSE;
	}
}



wstring OleUtils::getFileDisplayName(const wstring& filePath)
{
	IShellItem& item = getFileShellItem(filePath);
	LPWSTR buffer;
	HRESULT hres = item.GetDisplayName(SIGDN_NORMALDISPLAY, &buffer);
	wstring result = buffer;
	CoTaskMemFree(buffer);
	item.Release();
	throwOnFailure<OleException>(hres);
	return result;
}



IShellItem& OleUtils::getFileShellItem(const wstring& filePath)
{
	IShellItem* psiResult;
	LPITEMIDLIST pidlFilePath = getFilePidl(filePath);
	HRESULT hres = SHCreateItemFromIDList(pidlFilePath, IID_PPV_ARGS(&psiResult));
	CoTaskMemFree(pidlFilePath);
	throwOnFailure<OleException>(hres);
	return *psiResult;
}



IDataObject& OleUtils::getFileDataObject(const wstring& filePath)
{
	LPITEMIDLIST pidlFilePath = getFilePidl(filePath);
	return getFileDataObjectFromPidl(pidlFilePath);
}



IDataObject& OleUtils::getFileDataObjectWithIcon(const wstring& filePath)
{
	IDataObject& dataObject = getFileDataObject(filePath);

	SHFILEINFO sfi;
	SHDRAGIMAGE shdi;
	auto sysImageList = (HIMAGELIST) throwIfZero<OleException>(
		SHGetFileInfo(filePath.data(), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX));
	IconBitmap ib(sysImageList, sfi.iIcon);
	ib.fillSHDragImage(&shdi);

	DragHelper helper;
	helper.pdsh->SetFlags(DSH_ALLOWDROPDESCRIPTIONTEXT);
	HRESULT hres = helper.pdsh->InitializeFromBitmap(&shdi, &dataObject);
	if (FAILED(hres))
	{
		dataObject.Release();
		throw OleException(hres);
	}
	return dataObject;
}



vector<wstring> OleUtils::UnpackFileDataObject(IDataObject& data)
{
	// Fill STGMEDIUM structure.
	STGMEDIUM medium = { 0 };
	FORMATETC format = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	if (FAILED(data.GetData(&format, &medium)))
		return {};

	vector<wstring> result;
	HGLOBAL hg = medium.hGlobal;
	auto hDrop = (HDROP) GlobalLock(hg);
	if (hDrop != NULL)
	{
		result = UnpackHDrop(hDrop);
		GlobalUnlock(hg);
	}
	if (medium.pUnkForRelease == NULL)
		GlobalFree(hg);
	return result;
}



namespace {
	wstring makeAbsolutePath(const wstring& path)
	{
		TCHAR input[MAX_PATH], output[MAX_PATH];
		if (PathIsRoot(path.data()) != FALSE)
		{
			throwOnFailure<OleException>(
				StringCchCopy(input, MAX_PATH, path.data()));
		}
		else {
			TCHAR cwd[MAX_PATH];
			throwIfZero<OleException>(
				GetCurrentDirectory(MAX_PATH, cwd));
			throwIfZero<OleException, LPTSTR>(
				PathCombine(input, cwd, path.data()));
		}
		throwIfZero<OleException>(
			PathCanonicalize(output, input));
		return output;
	}



	LPITEMIDLIST getFilePidl(const wstring& filePath)
	{
		// Get non-const copy of const string.
		TCHAR pathBuffer[MAX_PATH];
		throwOnFailure<OleException>(
			StringCchCopy(pathBuffer, MAX_PATH, filePath.data()));

		ITEMIDLIST_RELATIVE* pidlFilePath;
		IShellFolder& desktop = getDesktopShellFolder();
		HRESULT hres = desktop.ParseDisplayName(
			0, NULL, pathBuffer, NULL, &pidlFilePath, 0);
		desktop.Release();
		throwOnFailure<OleException>(hres);
		return pidlFilePath;
	}



	IDataObject& getFileDataObjectFromPidl(LPCITEMIDLIST pidlFilePath)
	{
		if (pidlFilePath == NULL)
			throw OleException(FWP_E_NULL_POINTER);

		// Retrieve the file's parent folder.
		LPITEMIDLIST pidlParentFolder = ILClone(pidlFilePath);
		ILRemoveLastID(pidlParentFolder);
		IShellFolder& parentFolder = getDirShellFolder(pidlParentFolder);
		CoTaskMemFree(pidlParentFolder);

		// Retrieve the file's data object.
		LPCITEMIDLIST pidlFileName = ILFindLastID(pidlFilePath);
		IDataObject* pdoResult;
		throwOnFailure<OleException>(
			parentFolder.GetUIObjectOf(
			0, 1, &pidlFileName, IID_IDataObject, 0, (LPVOID*)&pdoResult));

		parentFolder.Release();
		return *pdoResult;
	}



	IShellFolder& getDirShellFolder(LPCITEMIDLIST pidlDirPath)
	{
		IShellFolder& desktop = getDesktopShellFolder();
		IShellFolder* psfFolder;
		HRESULT hres = desktop.BindToObject(
			pidlDirPath, NULL, IID_PPV_ARGS(&psfFolder));
		desktop.Release();
		throwOnFailure<OleException>(hres);
		return *psfFolder;
	}



	IShellFolder& getDesktopShellFolder()
	{
		IShellFolder* psfDesktop;
		throwOnFailure<OleException>(
			SHGetDesktopFolder(&psfDesktop));
		return *psfDesktop;
	}



	vector<wstring> UnpackHDrop(HDROP dropData)
	{
		int fileCount = DragQueryFile(dropData, 0xFFFFFFFF, 0, 0);
		vector<wstring> result;
		result.reserve(fileCount);
		TCHAR fileName[MAX_PATH];
		for (int i = 0; i < fileCount; ++i)
		{
			DragQueryFile(dropData, i, fileName, MAX_PATH);
			result.push_back(fileName);
		}
		return result;
	}



	DragHelper::DragHelper() : pdsh(NULL)
	{
		throwOnFailure<OleException>(
			CoCreateInstance(
			CLSID_DragDropHelper, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pdsh)));
	}

	DragHelper::~DragHelper()
	{
		if (pdsh != NULL)
			pdsh->Release();
	}
}
