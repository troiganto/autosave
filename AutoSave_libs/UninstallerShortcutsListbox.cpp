#include "stdafx.h"
#include "UninstallerShortcutsListbox.h"



HRESULT STDMETHODCALLTYPE UninstallerShortcutsListbox::QueryInterface(
	REFIID iid, void** ppvObject)
{
	if (!ppvObject)
		return E_INVALIDARG;
	*ppvObject = NULL;

	if (iid == IID_IUnknown || iid == IID_IDropTarget)
	{
		*ppvObject = (LPVOID) this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE UninstallerShortcutsListbox::AddRef()
{
	InterlockedIncrement(&m_cRef);
	return m_cRef;
}

ULONG STDMETHODCALLTYPE UninstallerShortcutsListbox::Release()
{
	ULONG refCount = InterlockedDecrement(&m_cRef);
	if (m_cRef == 0)
		delete this;
	return refCount;
}



void UninstallerShortcutsListbox::connect(HWND hwndParent, HWND hwndListbox)
{
	if (m_listbox == 0)
	{
		m_dialogbox = hwndParent;
		m_listbox = hwndListbox;
		m_tooltip.init(hwndParent, hwndListbox);
		SetWindowSubclass(hwndListbox, listboxSubclassProc, 0, (DWORD_PTR) this);
		SetWindowSubclass(hwndListbox,
			GdiUtils::subclassProcForListboxWithEmptyText,
			0, (DWORD_PTR) m_emptyText);
		RegisterDragDrop(hwndListbox, this);
		try {
			append(ShortcutsDisconnector::findShortcuts());
		}
		catch (AutoSaveException& exc) {
			m_lastException = exc;
			PostMessage(m_dialogbox, LB_EXCEPTIONTHROWN, 0, 0);
		}
	}
}



HRESULT STDMETHODCALLTYPE UninstallerShortcutsListbox::Drop(
	IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	vector<wstring> droppedFiles = OleUtils::UnpackFileDataObject(*pDataObj);
	bool hasAddedFiles = false;
	try {
		hasAddedFiles = append(droppedFiles);
	}
	catch (AutoSaveException& exc) {
		m_lastException = exc;
		PostMessage(m_dialogbox, LB_EXCEPTIONTHROWN, 0, 0);
		return exc.hResult();
	}
	catch (...) {
		// We must never throw!
		m_lastException = E_FAIL;
		PostMessage(m_dialogbox, LB_EXCEPTIONTHROWN, 0, 0);
		return E_FAIL;
	}
	if (hasAddedFiles)
	{
		PostMessage(m_dialogbox, LB_REFRESH, 0, 0);
	}
	else {
		m_tooltip.show(pt);
	}
	return S_OK;
}



LRESULT UninstallerShortcutsListbox::listboxSubclassProc(HWND hwnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam, UINT_PTR scid, DWORD_PTR refData)
{
	if (uMsg == WM_KEYDOWN && wParam == VK_DELETE)
	{
		auto pThis = (UninstallerShortcutsListbox*)refData;
		UINT cErasedFiles = pThis->eraseSelected();
		if (cErasedFiles > 0)
			PostMessage(pThis->m_dialogbox, LB_REFRESH, 0, 0);
		return 0;
	}
	else if (uMsg == WM_TIMER)
	{
		auto pThis = (UninstallerShortcutsListbox*)refData;
		pThis->m_tooltip.hide();
		return 0;
	}
	else {
		return DefSubclassProc(hwnd, uMsg, wParam, lParam);
	}
}



bool UninstallerShortcutsListbox::append(const vector<wstring>& newFiles)
{
	size_t oldSize = m_files.size();
	for (const wstring& file : newFiles)
	{
		if (ConnectedShortcut::isConnected(file) && !contains(file))
		{
			m_files.push_back(file);
		}
		else if (PathIsDirectory(file.data()) != FALSE)
		{
			appendDirectory(file);
		}
	}
	bool hasGrown = m_files.size() - oldSize != 0;
	if (hasGrown)
		updateListbox();
	return hasGrown;
}



void UninstallerShortcutsListbox::appendDirectory(const wstring& dir)
{
	vector<wstring> files =
		ShortcutsDisconnector::findConnectedShortcutsInFolder(dir);
	for (const wstring& file : files)
	{
		if (!contains(file))
			m_files.push_back(file);
	}
}



UINT UninstallerShortcutsListbox::eraseSelected()
{
	UINT itemCount = ListBox_GetCount(m_listbox);
	TCHAR buffer[MAX_PATH];
	for (UINT i = 0; i < itemCount; ++i)
	{
		if (ListBox_GetSel(m_listbox, i) > 0)
		{
			ListBox_GetText(m_listbox, i, buffer);
			erase(buffer);
		}
	}
	UINT diff = itemCount - (UINT) m_files.size();
	if (diff > 0)
		updateListbox();
	return diff;
}



bool UninstallerShortcutsListbox::contains(const wstring& file) const
{
	for (const wstring& f : m_files)
	{
		if (_tcsicmp(f.data(), file.data()) == 0)
			return true;
	}
	return false;
}



void UninstallerShortcutsListbox::erase(const wstring& file)
{
	for (auto pItem = m_files.begin(); pItem < m_files.end(); ++pItem)
	{
		if (_tcsicmp(pItem->data(), file.data()) == 0)
		{
			m_files.erase(pItem);
			return;
		}
	}
}



void UninstallerShortcutsListbox::updateListbox() const
{
	SendMessage(m_listbox, WM_SETREDRAW, FALSE, 0);
	ListBox_ResetContent(m_listbox);
	size_t maxWidth = 0;
	for (const wstring& file : m_files)
	{
		ListBox_AddString(m_listbox, file.data());
		maxWidth = __max(maxWidth, file.size());
	}
	ListBox_SetHorizontalExtent(m_listbox, getTextWidth(m_dialogbox, maxWidth));
	SendMessage(m_listbox, WM_SETREDRAW, TRUE, 0);
	RedrawWindow(m_listbox, NULL, NULL,
		RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}



int UninstallerShortcutsListbox::getTextWidth(
	HWND hwndDialogbox, const wstring& text)
{
	return getTextWidth(hwndDialogbox, text.size());
}

int UninstallerShortcutsListbox::getTextWidth(HWND hwndDialogbox, size_t cchText)
{
	RECT extent = { 0, 0, (LONG) cchText, 1 };
	MapDialogRect(hwndDialogbox, &extent);
	return (1 + extent.right) * 4;
}



