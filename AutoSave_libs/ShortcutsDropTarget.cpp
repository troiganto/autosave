#include "stdafx.h"
#include "ShortcutsDropTarget.h"


void ShortcutsDropTarget::registerTarget(HWND hwndTarget)
{
	if (!isRegistered())
	{
		m_hwnd = hwndTarget;
		RegisterDragDrop(m_hwnd, this);
	}
}

void ShortcutsDropTarget::revoke()
{
	if (isRegistered())
	{
		RevokeDragDrop(m_hwnd);
		m_hwnd = 0;
	}
}



HRESULT STDMETHODCALLTYPE ShortcutsDropTarget::QueryInterface(
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

ULONG STDMETHODCALLTYPE ShortcutsDropTarget::AddRef()
{
	InterlockedIncrement(&m_cRef);
	return m_cRef;
}

ULONG STDMETHODCALLTYPE ShortcutsDropTarget::Release()
{
	ULONG refCount = InterlockedDecrement(&m_cRef);
	if (m_cRef == 0)
	{
		delete this;
	}
	return refCount;
}



HRESULT STDMETHODCALLTYPE ShortcutsDropTarget::DragEnter(
	IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	vector<wstring> draggedFiles = OleUtils::UnpackFileDataObject(*pDataObj);
	m_draggedFile = getFirstFile(draggedFiles, L"");
	*pdwEffect = m_draggedFile.empty() ? DROPEFFECT_NONE : DROPEFFECT_LINK;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE ShortcutsDropTarget::DragLeave()
{
	m_draggedFile.clear();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE ShortcutsDropTarget::DragOver(
	DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	*pdwEffect = m_draggedFile.empty() ? DROPEFFECT_NONE : DROPEFFECT_LINK;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE ShortcutsDropTarget::Drop(
	IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	if (m_draggedFile.empty())
	{
		*pdwEffect = DROPEFFECT_NONE;
	}
	else {
		m_droppedFile = m_draggedFile;
		m_draggedFile.clear();
		*pdwEffect = DROPEFFECT_LINK;
		PostMessage(m_hwnd, DDM_FILEDROPPED,
			0, (LPARAM) m_droppedFile.data());
	}
	return S_OK;
}



const wstring& ShortcutsDropTarget::getFirstFile(
	const vector<wstring>& files, const wstring& defaultValue)
{
	for (const wstring& file : files)
	{
		if (PathIsDirectory(file.data()) == FALSE)
			return file;
	}
	return defaultValue;
}




