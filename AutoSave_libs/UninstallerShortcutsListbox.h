// UninstallerShortcutsListbox.h : Manages the listbox on the
// Connected Shortcuts page of the Uninstall wizard.
// Instead of throwing messages, this class posts
// UninstallerShortcutsListbox::LB_EXCEPTIONTHROWN messages to the
// dialog and sets its lastException member accordingly.


#pragma once

#include "stdafx.h"
#include "GdiUtils.h"
#include "OleUtils.h"
#include "ShortcutsDisconnector.h"
#include "UninstallerShortcutsListTooltip.h"

using std::vector;
using std::wstring;

class UninstallerShortcutsListbox : public IDropTarget
{
public:
	// Implementation of IUnknown
	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// Implementation of IDropTarget
	STDMETHOD(DragEnter)(
		IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
		*pdwEffect = DROPEFFECT_COPY;
		return S_OK;
	}
	STDMETHOD(DragLeave)() { return S_OK; }
	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
		*pdwEffect = DROPEFFECT_COPY;
		return S_OK;
	}
	STDMETHOD(Drop)(
		IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

public:
	enum {
		LB_REFRESH = WM_USER + 0x0010,
		LB_EXCEPTIONTHROWN
	};

	UninstallerShortcutsListbox() : m_cRef(1), m_listbox(0) {}
	~UninstallerShortcutsListbox() { RevokeDragDrop(m_listbox); }

	// If anything inside this object throws an exception,
	// the lastException member will be set and a message will be posted
	// to hwndParent.
	void connect(HWND hwndParent, HWND hwndListbox);
	const vector<wstring>& files = m_files;
	const HWND& listbox = m_listbox;
	const AutoSaveException& lastException = m_lastException;

protected:
	static LRESULT CALLBACK listboxSubclassProc(HWND hwnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam, UINT_PTR scid, DWORD_PTR refData);

	// Can only throw if ConnectedShortcut::isConnected throws.
	bool append(const vector<wstring>& newFiles);
	void appendDirectory(const wstring& dir);

	// These functions don't throw.
	UINT eraseSelected();
	bool contains(const wstring& file) const;
	void erase(const wstring& file);

	void updateListbox() const;
	static int getTextWidth(HWND hwndDialogbox, const wstring& text);
	static int getTextWidth(HWND hwndDialogbox, size_t cchText);

private:
	ULONG m_cRef;
	HWND m_dialogbox;
	HWND m_listbox;
	UninstallerShortcutsListTooltip m_tooltip;
	vector<wstring> m_files;
	AutoSaveException m_lastException;
	LPCTSTR m_emptyText = L"Couldn't find any connected shortcuts.\n"
		L"Please continue by clicking Next.";

};


