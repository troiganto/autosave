#include "stdafx.h"
#include "ShortcutsDisconnector.h"


ShortcutsDisconnector::ShortcutsDisconnector()
	: m_cRef(1), m_filesToBeRenamed(0)
{
	HRESULT hres;
	hres = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL,
		IID_PPV_ARGS(&m_pRenameOperation));
	if (FAILED(hres))
	{
		m_pRenameOperation = NULL;
		throw OleException(hres);
	}

	hres = m_pRenameOperation->Advise(
		(IFileOperationProgressSink*) this, &m_adviseCookie);
	if (FAILED(hres))
	{
		m_pRenameOperation->Release();
		m_pRenameOperation = NULL;
		throw OleException(hres);
	}
}



ShortcutsDisconnector::~ShortcutsDisconnector()
{
	if (m_pRenameOperation != NULL)
	{
		m_pRenameOperation->Unadvise(m_adviseCookie);
		m_pRenameOperation->Release();
	}
}



// IUnknown implementation
HRESULT STDMETHODCALLTYPE ShortcutsDisconnector::QueryInterface(
	REFIID iid, void** ppvObject)
{
	if (!ppvObject)
		return E_INVALIDARG;
	*ppvObject = NULL;

	if (iid == IID_IUnknown || iid == IID_IFileOperationProgressSink)
	{
		*ppvObject = (LPVOID) this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE ShortcutsDisconnector::AddRef()
{
	InterlockedIncrement(&m_cRef);
	return m_cRef;
}

ULONG STDMETHODCALLTYPE ShortcutsDisconnector::Release()
{
	ULONG refCount = InterlockedDecrement(&m_cRef);
	if (m_cRef == 0)
	{
		delete this;
	}
	return refCount;
}



// Here, we disconnect a successfully renamed shortcut.
HRESULT STDMETHODCALLTYPE ShortcutsDisconnector::PostRenameItem(
	DWORD dwFlags, IShellItem* psiItem, LPCWSTR pszNewName,
	HRESULT hrRename, IShellItem* psiNewlyCreated)
{
	if (FAILED(hrRename) || psiNewlyCreated == NULL)
		return hrRename;

	LPWSTR filePath;
	HRESULT hres = psiNewlyCreated->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
	if (FAILED(hres))
		return hres;

	// This function must not throw.
	try {
		ConnectedShortcut::disconnect(filePath);
		hres = S_OK;
	}
	catch (OleException& exc) {
		hres = exc.hResult();
	}
	CoTaskMemFree(filePath);
	return hres;
}



void ShortcutsDisconnector::removeShortcuts(const vector<wstring>& files)
{
	for (const wstring& file : files)
	{
		if (!DeleteFile(file.data()) != FALSE)
		{
			DWORD error = GetLastError();
			if (error != ERROR_FILE_NOT_FOUND)
				throw OleException(error);
		}
	}
}



void ShortcutsDisconnector::disconnectShortcuts(const vector<wstring>& files)
{
	ShortcutsDisconnector scd;
	for (const wstring& file : files)
	{
		scd.addFile(file);
	}
	scd.performOperations();
}



void ShortcutsDisconnector::addFile(const wstring& file)
{
	if (declareRename(file))
	{
		// File must be renamed; count only successfully added files.
		++m_filesToBeRenamed;
	}
	else {
		// File doesn't have expected suffix: Just disconnect and leave.
		ConnectedShortcut::disconnect(file);
	}
}



bool ShortcutsDisconnector::declareRename(const wstring& filePath)
{
	wstring newName = ConnectedShortcut::disconnectFileName(filePath);
	// Check if name matches expectations.
	if (newName == filePath)
		return false;

	// Remove path to the name.
	newName = newName.substr(newName.find_last_of(L'\\') + 1);
	
	// Declare rename operation.
	IShellItem& item = OleUtils::getFileShellItem(filePath);
	HRESULT hres = m_pRenameOperation->RenameItem(&item, newName.data(), NULL);
	item.Release();
	throwOnFailure<OleException>(hres);
	return true;
}



// Rename files and find out whether the operation was successful.
void ShortcutsDisconnector::performOperations()
{
	if (m_filesToBeRenamed == 0)
		return;

	m_pRenameOperation->SetOperationFlags(FOF_FILESONLY | FOFX_NOMINIMIZEBOX);
	throwOnFailure<OleException>(m_pRenameOperation->PerformOperations());

	BOOL isAborted = FALSE;
	HRESULT hres = m_pRenameOperation->GetAnyOperationsAborted(&isAborted);
	throwOnFailure<OleException>(isAborted ? E_ABORT : hres);
}



vector<wstring> ShortcutsDisconnector::findShortcuts()
{
	vector<wstring> registeredFiles, desktopFiles;
	try {
		registeredFiles = findConnectedShortcutsInRegistry();
	}
	catch (RegistryException& exc) {
		// Silently ignore a non-existing registry value.
		if (exc.errorCode() != ERROR_FILE_NOT_FOUND)
			throw;
	}
	desktopFiles = findConnectedShortcutsOnDesktop();
	mergeLists(registeredFiles, desktopFiles);
	return registeredFiles;
}



vector<wstring> ShortcutsDisconnector::findConnectedShortcutsInRegistry()
{
	vector<wstring> registeredFiles, existingFiles;
	RegistryAccess ra;

	ra.access(DEFAULT_REGISTRY_KEY);
	registeredFiles = ra.readMultiString(CONNECTED_SHORTCUTS_REGISTRY_VALUE_NAME);
	ra.close();

	existingFiles.reserve(registeredFiles.size());
	for (const wstring& file : registeredFiles)
	{
		if (PathFileExists(file.data()))
			existingFiles.push_back(file);
	}

	return existingFiles;
}



vector<wstring> ShortcutsDisconnector::findConnectedShortcutsOnDesktop()
{
	vector<wstring> foundFiles;

	LPTSTR buffer;
	throwOnFailure<OleException>(
		SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &buffer));
	wstring desktopPath = buffer;
	desktopPath.push_back(L'\\');
	CoTaskMemFree(buffer);

	return findConnectedShortcutsInFolder(desktopPath);
}



vector<wstring> ShortcutsDisconnector::findConnectedShortcutsInFolder(wstring dir)
{
	if (dir.empty())
		throw OleException(E_INVALIDARG);
	if (dir.back() != L'\\')
		dir.push_back(L'\\');

	vector<wstring> foundFiles;
	WIN32_FIND_DATA hit;
	HANDLE search = FindFirstFile((dir + L"*.lnk").data(), &hit);
	if (search == INVALID_HANDLE_VALUE)
		throw OleException();

	do
	{
		wstring filePath = dir + hit.cFileName;
		if (ConnectedShortcut::isConnected(filePath))
			foundFiles.push_back(filePath);
	} while (FindNextFile(search, &hit) != FALSE);
	FindClose(search);
	return foundFiles;
}



// Copies donor's items into acceptor, while avoiding duplicates.
void ShortcutsDisconnector::mergeLists(
	vector<wstring>& acceptor, const vector<wstring>& donor)
{
	bool isInAcceptor;
	for (const wstring& donatedFile : donor)
	{
		isInAcceptor = std::find<vector<wstring>::iterator, wstring>(
			acceptor.begin(), acceptor.end(), donatedFile) != acceptor.end();
		if (!isInAcceptor)
			acceptor.push_back(donatedFile);
	}
}



void ShortcutsDisconnector::registerConnectedShortcuts(const wstring& file)
{
	try {
		if (!ConnectedShortcut::isConnected(file))
			return;
		RegistryAccess ra;
		ra.access(DEFAULT_REGISTRY_KEY);
		ra.AppendToMultiString(CONNECTED_SHORTCUTS_REGISTRY_VALUE_NAME, file);
		ra.close();
	}
	catch (std::runtime_error&) {}
}



