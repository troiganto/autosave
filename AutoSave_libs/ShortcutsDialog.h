// ShortcutsDialog.h : Implements the dialog that lets users
// create connected shortcuts.
// May only throw in the constructor.

#pragma once

#include "stdafx.h"
#include "OleUtils.h"
#include "GdiUtils.h"
#include "MiscSettings.h"
#include "TemporaryShortcutFile.h"
#include "ShortcutsDropTarget.h"
#include "..\AutoSave\Resource.h"

class ShortcutsDialog
{
public:
	~ShortcutsDialog();

	static INT_PTR show(HWND hwndParent, const MiscSettings& defaultSettings);

protected:
	static INT_PTR CALLBACK dialogProc(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL handleCommand(UINT code, UINT ctrlId, HWND ctrlHandle);
	BOOL handleNotify(const NMHDR* pHeader);

	void onInitDialog();
	void onDestroy();

	BOOL onButtonClick(UINT ctrlId, HWND ctrlHandle);
	void onEditChange(UINT ctrlId, HWND ctrlHandle);
	void onIntervalEditKillFocus();
	void onComboBoxChange(UINT ctrlId, HWND ctrlHandle);

	void onFileDrop(const wstring& fileName);
	void onDragResultOut(int mouseKey, const POINT& origin);
	void onLvSourceDoubleClick();
	void onLvResultDoubleClick();
	void onShortcutSuccessfullyCreated();

private:
	ShortcutsDialog(const MiscSettings& defaultSettings);

	// Never throws.
	int getConfigMaskFromForm() const;

	// Fail silently.
	static const wstring openFileDialog(HWND hwndParent);
	static const wstring saveFileDialog(HWND hwndParent, const wstring& suggestedName);
	static const wstring GetFileDialogResult(IFileDialog* pfdDialog);

	// Never throw.
	void initListview(HWND lv, LPCTSTR emptyText);
	static void showFileInListview(const wstring& fileName, HWND lv);
	static LRESULT CALLBACK listviewSubclassProc(
		HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR scid, DWORD_PTR refData);
	static DWORD CALLBACK fillListviewsThread(LPVOID pArgs);
	struct FillListviewsThreadArgs {
		ShortcutsDialog* pThis;
		const wstring fileName;
	};

	// Other components
	ShortcutsDropTarget m_dropTarget;
	TemporaryShortcutFile m_shortcut;

	HWND m_hwnd;
	bool m_firstShortcutSuccessfullyCreated;
	bool m_useCustomSettings;
	const MiscSettings& m_DefaultSettings;
	MiscSettings m_customSettings;

	const static int m_emptyTextMaxSize = 128;
	wchar_t m_lvSourceEmptyText[m_emptyTextMaxSize];
	wchar_t m_lvResultEmptyText[m_emptyTextMaxSize];

};