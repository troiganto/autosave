// ShortcutsDropTarget.h : Implements the IDropTarget interface
// for the Create Connected Shortcut dialog.
// Never throws since OleUtils::UnpackFileDataObject doesn't throw.

#pragma once

#include "stdafx.h"
#include "OleUtils.h"

class ShortcutsDropTarget : IDropTarget
{
public:
	// Implementation of IUnknown
	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// Implementation of IDropTarget
	STDMETHOD(DragEnter)(
		IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	STDMETHOD(DragLeave)();
	STDMETHOD(DragOver)(
		DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	STDMETHOD(Drop)(
		IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

public:
	ShortcutsDropTarget() : m_cRef(1) {}
	~ShortcutsDropTarget() { revoke(); }

	void registerTarget(HWND hwndTarget);
	void revoke();
	inline bool isRegistered() { return m_hwnd != 0; }

	static const UINT DDM_FILEDROPPED = WM_USER + 0x10;

private:
	static const wstring& getFirstFile(
		const vector<wstring>& files, const wstring& defaultValue);

	HWND m_hwnd;
	wstring m_droppedFile;
	wstring m_draggedFile;

	ULONG m_cRef;
};

