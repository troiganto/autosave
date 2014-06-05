// ShortcutsDisconnector.h: Disconnects, deletes, and /finds/ connected shortcuts.
// Uses Shell file operations to do the renaming as gracefully as possible.
// Usually throws OleException on failure. Only throws RegistryException
// when RegistryAccess does.
// The registerConnectedShortcut function fails silently.

#pragma once

#include "stdafx.h"
#include "OleUtils.h"
#include "RegistryAccess.h"
#include "ConnectedShortcut.h"

class ShortcutsDisconnector : public IFileOperationProgressSink
{
public:
	// Lots of inline implementations. The only interesting stuff
	// are PostRenameItem and everything below this god-awful block.

	// Implementation of IUnknown
	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// Implementation of IFileOperationProgressSink
	STDMETHOD(FinishOperations)(HRESULT hrResult) { return S_OK; }
	STDMETHOD(PauseTimer)() { return S_OK; }
	STDMETHOD(PostCopyItem)(DWORD dwFlags, IShellItem* psiItem,
		IShellItem* psiDestinationFolder, LPCWSTR pszNewName, HRESULT hrCopy,
		IShellItem* psiNewlyCreated) { return hrCopy; }
	STDMETHOD(PostDeleteItem)(DWORD dwFlags, IShellItem* psiItem, HRESULT hrDelete,
		IShellItem* psiNewlyCreated) { return hrDelete; }
	STDMETHOD(PostMoveItem)(DWORD dwFlags, IShellItem* psiItem,
		IShellItem* psiDestinationFolder, LPCWSTR pszNewName, HRESULT hrMove,
		IShellItem* psiNewlyCreated) { return hrMove; }
	STDMETHOD(PostNewItem)(DWORD dwFlags, IShellItem* psiDestinationFolder,
		LPCWSTR pszNewName, LPCWSTR pszTemplateName, DWORD dwFileAttributes,
		HRESULT hrNew, IShellItem* psiNewItem) { return hrNew; }

	// No inline implementation!
	STDMETHOD(PostRenameItem)(DWORD dwFlags, IShellItem* psiItem, LPCWSTR pszNewName,
		HRESULT hrRename, IShellItem* psiNewlyCreated);

	STDMETHOD(PreCopyItem)(DWORD dwFlags, IShellItem* psiItem,
		IShellItem* psiDestinationFolder, LPCWSTR pszNewName) { return S_OK; }
	STDMETHOD(PreDeleteItem)(DWORD dwFlags, IShellItem* psiItem) { return S_OK; }
	STDMETHOD(PreMoveItem)(DWORD dwFlags, IShellItem* psiItem,
		IShellItem* psiDestinationFolder, LPCWSTR pszNewName) { return S_OK; }
	STDMETHOD(PreNewItem)(DWORD dwFlags, IShellItem* psiDestinationFolder,
		LPCWSTR pszNewName) { return S_OK; }
	STDMETHOD(PreRenameItem)(DWORD dwFlags, IShellItem* psiItem, LPCWSTR pszNewName) {
		return S_OK; }
	STDMETHOD(ResetTimer)() { return S_OK; }
	STDMETHOD(ResumeTimer)() { return S_OK; }
	STDMETHOD(StartOperations)() { return S_OK; }
	STDMETHOD(UpdateProgress)(UINT iWorkTotal, UINT iWorkSoFar) { return S_OK; }

public:
	~ShortcutsDisconnector();
	static void removeShortcuts(const vector<wstring>& files);
	static void disconnectShortcuts(const vector<wstring>& files);

	static vector<wstring> findShortcuts();
	static vector<wstring> findConnectedShortcutsInRegistry();
	static vector<wstring> findConnectedShortcutsOnDesktop();
	static vector<wstring> findConnectedShortcutsInFolder(wstring dir);

	static void registerConnectedShortcuts(const wstring& file);

private:
	ShortcutsDisconnector();

	void addFile(const wstring& file);
	bool declareRename(const wstring& filePath);
	void performOperations();

	static void mergeLists(
		vector<wstring>& acceptor, const vector<wstring>& donor);

	ULONG m_cRef;
	UINT m_filesToBeRenamed;
	IFileOperation* m_pRenameOperation;
	DWORD m_adviseCookie;
};

