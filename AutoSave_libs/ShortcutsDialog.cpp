#include "stdafx.h"
#include "ShortcutsDialog.h"


ShortcutsDialog::ShortcutsDialog(const MiscSettings& defaultSettings)
	: m_firstShortcutSuccessfullyCreated(false),
	  m_DefaultSettings(defaultSettings),
	  m_customSettings(defaultSettings),
	  m_useCustomSettings(false)
{
	LoadString(GetModuleHandle(NULL), IDS_SLV_EMPTYSOURCE,
		m_lvSourceEmptyText, m_emptyTextMaxSize);
	LoadString(GetModuleHandle(NULL), IDS_SLV_EMPTYRESULT,
		m_lvResultEmptyText, m_emptyTextMaxSize);
}


ShortcutsDialog::~ShortcutsDialog()
{
	// Destroy the dialog, should this ever go out of scope.
	if (IsWindow(m_hwnd))
		EndDialog(m_hwnd, IDCANCEL);
}



// Returns IDABORT if AutoSave should exit after calling this function.
// Otherwise, it return IDCANCEL.
INT_PTR ShortcutsDialog::show(HWND hwndParent, const MiscSettings& defaultSettings)
{
	ShortcutsDialog sd(defaultSettings);

	return DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SHORTCUTS),
		hwndParent, dialogProc, (LPARAM) &sd);
}


INT_PTR CALLBACK ShortcutsDialog::dialogProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ShortcutsDialog* pThis = NULL;

	if (uMsg == WM_INITDIALOG)
	{
		pThis = (ShortcutsDialog*) lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)pThis);
		// Set the m_hwnd member as early as possible.
		pThis->m_hwnd = hwndDlg;
	}
	else {
		pThis = (ShortcutsDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	}

	if (pThis)
	{
		return pThis->handleMessage(uMsg, wParam, lParam);
	}
	else {
		return FALSE;
	}
}



BOOL ShortcutsDialog::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	try {
		if (uMsg == WM_INITDIALOG)
		{
			onInitDialog();
			return TRUE;
		}
		else if (uMsg == WM_COMMAND)
		{
			return handleCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		}
		else if (uMsg == ShortcutsDropTarget::DDM_FILEDROPPED)
		{
			onFileDrop((LPTSTR)lParam);
			return TRUE;
		}
		else if (uMsg == WM_NOTIFY)
		{
			return handleNotify((NMHDR*)lParam);
		}
		else {
			return FALSE;
		}
	}
	catch (AutoSaveException& exc) {
		exc.showMessageBox(m_hwnd, L"An unexpected error occured.");
		return FALSE;
	}
	catch (std::exception& exc) {
		AutoSaveException::showMessageBox(
			m_hwnd, L"An unexpected error occured.", exc);
		return FALSE;
	}
}

BOOL ShortcutsDialog::handleCommand(UINT code, UINT ctrlId, HWND ctrlHandle)
{
	if (code == BN_CLICKED)
	{
		return onButtonClick(ctrlId, ctrlHandle);
	}
	else if (code == EN_CHANGE)
	{
		onEditChange(ctrlId, ctrlHandle);
		return TRUE;
	}
	else if (code == CBN_SELENDOK)
	{
		onComboBoxChange(ctrlId, ctrlHandle);
		return TRUE;
	}
	else if (code == EN_KILLFOCUS && ctrlId == IDC_SHORTCUTS_INTERVALEDIT)
	{
		onIntervalEditKillFocus();
		return TRUE;
	}
	else {
		return FALSE;
	}
}



BOOL ShortcutsDialog::handleNotify(const NMHDR* pHeader)
{
	if (pHeader->idFrom == IDC_SHORTCUTS_SOURCE)
	{
		if (pHeader->code == NM_DBLCLK)
		{
			onLvSourceDoubleClick();
			return TRUE;
		}
		else if (pHeader->code == LVN_KEYDOWN &&
			VK_RETURN == ((NMLVKEYDOWN*) pHeader)->wVKey)
		{
			// We don't use NM_RETURN here because it doesn't get sent
			// when the list-view is empty.
			onLvSourceDoubleClick();
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else if (pHeader->idFrom == IDC_SHORTCUTS_RESULT)
	{
		if (pHeader->code == NM_DBLCLK || pHeader->code == NM_RETURN)
		{
			// This list-view always has an item when it matters,
			// so we can just use NM_RETURN
			onLvResultDoubleClick();
			return TRUE;
		}
		else if (pHeader->code == LVN_BEGINDRAG || pHeader->code == LVN_BEGINRDRAG)
		{
			int button = (pHeader->code == LVN_BEGINRDRAG) ? MK_RBUTTON : MK_LBUTTON;
			const POINT& origin = ((LPNMLISTVIEW) pHeader)->ptAction;
			onDragResultOut(button, origin);
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}



void ShortcutsDialog::onInitDialog()
{
	// Enable drag&drop
	m_dropTarget.registerTarget(m_hwnd);

	// Change control properties.
	initListview(GetDlgItem(m_hwnd, IDC_SHORTCUTS_SOURCE), m_lvSourceEmptyText);
	initListview(GetDlgItem(m_hwnd, IDC_SHORTCUTS_RESULT), m_lvResultEmptyText);

	Edit_LimitText(GetDlgItem(m_hwnd, IDC_SHORTCUTS_INTERVALEDIT), 5);
	SendDlgItemMessage(m_hwnd, IDC_SHORTCUTS_INTERVALUPDOWN, UDM_SETRANGE32,
		m_DefaultSettings.getMinInterval(),
		m_DefaultSettings.getMaxInterval());

	// Fill in verbosity list.
	// Length = length of longest text + 30%.
	TCHAR listEntry[64];
	HWND hList = GetDlgItem(m_hwnd, IDC_SHORTCUTS_VERBOSITYLIST);
	for (int i = IDS_VERBOSITY_DESC0; i <= IDS_VERBOSITY_DESC3; ++i)
	{
		LoadString(GetModuleHandle(NULL), i, listEntry, 64);
		ComboBox_AddString(hList, listEntry);
	}

	// Fill in defaults.
	SendDlgItemMessage(m_hwnd, IDC_SHORTCUTS_HOTKEYEDIT,
		HKM_SETHOTKEY, m_DefaultSettings.getHotkey(), 0);
	SetDlgItemInt(m_hwnd, IDC_SHORTCUTS_INTERVALEDIT,
		m_DefaultSettings.getInterval(), FALSE);
	ComboBox_SetCurSel(hList, (int) m_DefaultSettings.getVerbosity());
}



void ShortcutsDialog::onDestroy()
{
	
}



BOOL ShortcutsDialog::onButtonClick(UINT ctrlId, HWND ctrlHandle)
{
	if (ctrlId == IDCANCEL)
	{
		INT_PTR result = Button_GetCheck(GetDlgItem(m_hwnd,
			IDC_SHORTCUTS_SHUTDOWNCHECK)) == BST_CHECKED ?
			IDABORT : IDCLOSE;
		onDestroy();
		EndDialog(m_hwnd, result);
		return TRUE;
	}
	else if (ctrlId == IDC_SHORTCUTS_CUSTOMCHECK)
	{
		m_useCustomSettings = Button_GetCheck(ctrlHandle) == BST_CHECKED;
		int ids[] = { IDC_SHORTCUTS_HOTKEYCHECK, IDC_SHORTCUTS_HOTKEYEDIT,
			IDC_SHORTCUTS_INTERVALCHECK, IDC_SHORTCUTS_INTERVALEDIT,
			IDC_SHORTCUTS_INTERVALUPDOWN, IDC_SHORTCUTS_VERBOSITYCHECK,
			IDC_SHORTCUTS_VERBOSITYLIST };
		for (int id : ids)
		{
			EnableWindow(GetDlgItem(m_hwnd, id), m_useCustomSettings);
		}
		return TRUE;
	}
	else {
		return FALSE;
	}
}



void ShortcutsDialog::onEditChange(UINT ctrlId, HWND ctrlHandle)
{
	if (ctrlId == IDC_SHORTCUTS_HOTKEYEDIT)
	{
		LRESULT content = SendMessage(ctrlHandle, HKM_GETHOTKEY, 0, 0);
		m_customSettings.setHotkey(LOWORD(content));
		if (m_customSettings.getHotkey() != m_DefaultSettings.getHotkey())
			CheckDlgButton(m_hwnd, IDC_SHORTCUTS_HOTKEYCHECK, BST_CHECKED);
	}
	else if (ctrlId == IDC_SHORTCUTS_INTERVALEDIT)
	{
		m_customSettings.setInterval(GetDlgItemInt(m_hwnd, ctrlId, NULL, FALSE));
		if (m_customSettings.getInterval() != m_DefaultSettings.getInterval())
			CheckDlgButton(m_hwnd, IDC_SHORTCUTS_INTERVALCHECK, BST_CHECKED);
	}
}



// See OptionsPageMore::onIntervalEditKillFocus on what's going in here.
void ShortcutsDialog::onIntervalEditKillFocus()
{
	SetDlgItemInt(m_hwnd, IDC_SHORTCUTS_INTERVALEDIT,
		m_customSettings.getInterval(), FALSE);
}



void ShortcutsDialog::onComboBoxChange(UINT ctrlId, HWND ctrlHandle)
{
	int selection ComboBox_GetCurSel(ctrlHandle);
	m_customSettings.setVerbosity(selection);
	if (selection != m_DefaultSettings.getVerbosity())
		CheckDlgButton(m_hwnd, IDC_SHORTCUTS_VERBOSITYCHECK, BST_CHECKED);
}



void ShortcutsDialog::onFileDrop(const wstring& fileName)
{
	auto pArgs = new FillListviewsThreadArgs{ this, fileName };
	CloseHandle(CreateThread(NULL, 12, fillListviewsThread, pArgs, 0, NULL));
}



void ShortcutsDialog::onDragResultOut(int mouseKey, const POINT& origin)
{

	const int configMask = getConfigMaskFromForm();
	m_shortcut.updateSettings(m_customSettings, configMask);

	IDataObject& dropData = OleUtils::getFileDataObjectWithIcon(
		m_shortcut.getFile());

	DWORD dragOperation = 0;
	HRESULT dragDropResult = SHDoDragDrop(
		GetDlgItem(m_hwnd, IDC_SHORTCUTS_RESULT), &dropData,
		NULL, DROPEFFECT_MOVE, &dragOperation);
	dropData.Release();
	throwOnFailure<OleException>(dragDropResult);

	// Adding <&& dragOperation != DROPEFFECT_NONE> didn't work.
	if (dragDropResult == DRAGDROP_S_DROP)
		onShortcutSuccessfullyCreated();
}



void ShortcutsDialog::onLvSourceDoubleClick()
{
	wstring fileName = openFileDialog(m_hwnd);
	if (!fileName.empty())
	{
		onFileDrop(fileName);
	}
}



void ShortcutsDialog::onLvResultDoubleClick()
{
	const int configMask = getConfigMaskFromForm();
	m_shortcut.updateSettings(m_customSettings, configMask);

	wstring shortcutFile = m_shortcut.getFile();
	if (shortcutFile.empty())
		return;

	TCHAR filePath[MAX_PATH];
	StringCchCopy(filePath, MAX_PATH, shortcutFile.data());
	LPTSTR suggestedFile = PathFindFileName(filePath);
	PathRemoveExtension(suggestedFile);

	wstring fileName = saveFileDialog(m_hwnd, suggestedFile);
	if (fileName.empty())
		return;

	bool success = MoveFileEx(shortcutFile.data(), fileName.data(),
		MOVEFILE_REPLACE_EXISTING) != FALSE;
	if (success)
	{
		onShortcutSuccessfullyCreated();
	}
	else {
		MessageBox(m_hwnd, L"An error occured while trying to create the shortcut.",
			APP_NAME, MB_ICONERROR);
	}
}



void ShortcutsDialog::onShortcutSuccessfullyCreated()
{
	if (!m_firstShortcutSuccessfullyCreated)
	{
		m_firstShortcutSuccessfullyCreated = true;
		CheckDlgButton(m_hwnd, IDC_SHORTCUTS_SHUTDOWNCHECK, BST_CHECKED);
	}
}



int ShortcutsDialog::getConfigMaskFromForm() const
{
	// Shortcut
	bool useCustomConfig = BST_CHECKED == Button_GetCheck(
		GetDlgItem(m_hwnd, IDC_SHORTCUTS_CUSTOMCHECK));
	if (!useCustomConfig)
		return MiscSettings::ATT_NONE;

	// If a certain checkbox is enabled, its bit is OR'd to the mask.
	int mask = 0;
	vector<pair<UINT, int>> checkBoxes = {
			{ IDC_SHORTCUTS_HOTKEYCHECK, MiscSettings::ATT_HOTKEY },
			{ IDC_SHORTCUTS_INTERVALCHECK, MiscSettings::ATT_INTERVAL },
			{ IDC_SHORTCUTS_VERBOSITYCHECK, MiscSettings::ATT_VERBOSITY }
	};
	for (pair<UINT, int> item : checkBoxes)
	{
		if (BST_CHECKED == Button_GetCheck(GetDlgItem(m_hwnd, item.first)))
			mask |= item.second;
	}

	return mask;
}



const wstring ShortcutsDialog::openFileDialog(HWND hwndParent)
{
	IFileDialog* pfdOpen;
	throwOnFailure<OleException>(
		CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_PPV_ARGS(&pfdOpen)));

	FILEOPENDIALOGOPTIONS flags;
	pfdOpen->GetOptions(&flags);
	pfdOpen->SetOptions(flags | FOS_NODEREFERENCELINKS | FOS_FORCEFILESYSTEM);

	COMDLG_FILTERSPEC fileTypes[2] = {
			{ L"Applications", L"*.exe" },
			{ L"All files", L"*.*" }
	};
	pfdOpen->SetFileTypes(2, fileTypes);
	pfdOpen->SetFileTypeIndex(1);

	wstring result = HRESULT_CODE(pfdOpen->Show(hwndParent)) != ERROR_CANCELLED ?
		GetFileDialogResult(pfdOpen) : L"";
	pfdOpen->Release();
	return result;
}



const wstring ShortcutsDialog::saveFileDialog(
	HWND hwndParent, const wstring& suggestedName)
{
	IFileDialog* pfdSave;
	throwOnFailure<OleException>(
		CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
			IID_PPV_ARGS(&pfdSave)));

	FILEOPENDIALOGOPTIONS flags;
	pfdSave->GetOptions(&flags);
	pfdSave->SetOptions(flags | FOS_STRICTFILETYPES | FOS_DONTADDTORECENT);

	SHFILEINFO fi;
	SHGetFileInfo(L"*.lnk", FILE_ATTRIBUTE_NORMAL, &fi, sizeof(fi),
		SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
	COMDLG_FILTERSPEC fileTypes[1] = {
			{ fi.szTypeName, L"*.lnk" },
	};
	pfdSave->SetFileTypes(1, fileTypes);

	pfdSave->SetFileName(suggestedName.data());

	wstring result = HRESULT_CODE(pfdSave->Show(hwndParent)) != ERROR_CANCELLED ?
		GetFileDialogResult(pfdSave) : L"";
	pfdSave->Release();
	if (!result.empty() && !OleUtils::isShortcutFile(result))
		result.append(L".lnk");
	return result;
}



const wstring ShortcutsDialog::GetFileDialogResult(IFileDialog* pfdDialog)
{
	wstring result;
	IShellItem* psiResult;
	if (SUCCEEDED(pfdDialog->GetResult(&psiResult)))
	{
		LPWSTR fileName;
		psiResult->GetDisplayName(SIGDN_FILESYSPATH, &fileName);
		result = fileName;
		CoTaskMemFree(fileName);
		psiResult->Release();
	}
	return result;
}



void ShortcutsDialog::initListview(HWND lv, LPCWSTR emptyText)
{
	ListView_SetView(lv, LV_VIEW_TILE);

	LVCOLUMN column = {
		LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, 0, 100, L"Display name", 5, 0 };
	ListView_InsertColumn(lv, 0, &column);

	column.iSubItem = 1;
	column.pszText = L"Type name";
	ListView_InsertColumn(lv, 1, &column);

	SetWindowSubclass(lv, GdiUtils::subclassProcForListviewWithEmptyText, 0,
		(DWORD_PTR) emptyText);
	SetWindowSubclass(lv, listviewSubclassProc, 1, 0);
}



void ShortcutsDialog::showFileInListview(const wstring& fileName, HWND lv)
{
	ListView_DeleteAllItems(lv);

	// Get file info
	SHFILEINFO finfo;
	auto hil = (HIMAGELIST)SHGetFileInfo(
		fileName.data(), 0, &finfo, sizeof(finfo),
		SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_SYSICONINDEX);
	ListView_SetImageList(lv, hil, LVSIL_NORMAL);

	// Add item (display name) and subitem (type name)
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_TEXT | LVIF_GROUPID | LVIF_IMAGE | LVIF_COLFMT;
	lvi.iGroupId = I_GROUPIDNONE;
	lvi.pszText = finfo.szDisplayName;
	lvi.iImage = finfo.iIcon;
	ListView_InsertItem(lv, &lvi);
	ListView_SetItemText(lv, 0, 1, finfo.szTypeName);

	// Make the second line appear
	UINT columnIndex = 1;
	LVTILEINFO ti = { sizeof(ti), 0, 1, &columnIndex, NULL };
	ListView_SetTileInfo(lv, &ti);
}



LRESULT CALLBACK ShortcutsDialog::listviewSubclassProc(
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR scid, DWORD_PTR refData)
{
	if (uMsg == WM_GETDLGCODE && wParam == VK_RETURN)
	{
		return DLGC_WANTMESSAGE;
	}
	else {
		return DefSubclassProc(hwnd, uMsg, wParam, lParam);
	}
}



// Loads the specified file and updates the temporary connected shortcut
// in a concurrent thread.
DWORD CALLBACK ShortcutsDialog::fillListviewsThread(LPVOID pArgs)
{
	HANDLE listViewMutex = CreateMutex(NULL, FALSE, APP_NAME L" Listview Mutex");
	if (listViewMutex != NULL)
	{
		DWORD waitResult = WaitForSingleObject(listViewMutex, 0);
		if (waitResult == WAIT_OBJECT_0 || waitResult == WAIT_ABANDONED)
		{
			// Get the passed arguments.
			ShortcutsDialog* pThis = ((FillListviewsThreadArgs*)pArgs)->pThis;
			const wstring& fileName = ((FillListviewsThreadArgs*)pArgs)->fileName;
			HWND lvSource = GetDlgItem(pThis->m_hwnd, IDC_SHORTCUTS_SOURCE);
			HWND lvResult = GetDlgItem(pThis->m_hwnd, IDC_SHORTCUTS_RESULT);

			// Show the Please Wait message.
			LoadString(GetModuleHandle(NULL), IDS_SLV_WAIT,
				pThis->m_lvSourceEmptyText, m_emptyTextMaxSize);
			LoadString(GetModuleHandle(NULL), IDS_SLV_WAIT,
				pThis->m_lvResultEmptyText, m_emptyTextMaxSize);
			ListView_DeleteAllItems(lvSource);
			ListView_DeleteAllItems(lvResult);

			// Load the given file.
			OleInitialize(NULL);
			showFileInListview(fileName, lvSource);

			// Update the connected shortcut and load it, too.
			try {
				pThis->m_shortcut.updateFile(fileName);
				showFileInListview(pThis->m_shortcut.getFile(), lvResult);
			}
			catch (AutoSaveException&) {
				LoadString(GetModuleHandle(NULL), IDS_SLV_SOURCEERROR,
					pThis->m_lvSourceEmptyText, m_emptyTextMaxSize);
				LoadString(GetModuleHandle(NULL), IDS_SLV_RESULTERROR,
					pThis->m_lvResultEmptyText, m_emptyTextMaxSize);
				InvalidateRect(lvSource, NULL, TRUE);
				InvalidateRect(lvResult, NULL, TRUE);
				UpdateWindow(lvSource);
				UpdateWindow(lvResult);
			}
			OleUninitialize();
			ReleaseMutex(listViewMutex);
		}
	}
	delete pArgs;
	CloseHandle(listViewMutex);
	return 0;
}



