#include "stdafx.h"
#include "OptionsPageMore.h"


OptionsPageMore::OptionsPageMore(MiscSettings* pSettings)
	: m_oldSettings(*pSettings), // This is a reference,
	  m_newSettings(m_oldSettings), // this is a copy.
	  m_shouldAppExit(false),
	  shouldAppExit(m_shouldAppExit)
{
}



OptionsPageMore::~OptionsPageMore()
{
}



HPROPSHEETPAGE OptionsPageMore::create()
{
	PROPSHEETPAGE psp = { 0 };
	psp.dwSize = sizeof(psp);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = GetModuleHandle(NULL);
	psp.pszTemplate = MAKEINTRESOURCE(IDD_SETTINGS_MORE);
	psp.pfnDlgProc = dialogProc;
	psp.lParam = (LPARAM) this;
	return CreatePropertySheetPage(&psp);
}



INT_PTR OptionsPageMore::handleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_INITDIALOG)
	{
		onInitDialog();
		return TRUE;
	}
	else if (message == WM_NOTIFY)
	{
		return handleNotify((NMHDR*)lParam);
	}
	else if (message == WM_COMMAND)
	{
		return handleCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
	}
	else {
		return FALSE;
	}
}



INT_PTR OptionsPageMore::handleCommand(UINT code, UINT ctrlId, HWND ctrlHandle)
{
	if (code == BN_CLICKED)
	{
		onButtonClick(ctrlId, ctrlHandle);
		return TRUE;
	}
	else if (code == EN_CHANGE)
	{
		onEditChange(ctrlId, ctrlHandle);
		updateHasChanged();
		return TRUE;
	}
	else if (code == CBN_SELENDOK)
	{
		onComboBoxChange(ctrlId, ctrlHandle);
		updateHasChanged();
		return TRUE;
	}
	else if (code == EN_KILLFOCUS && ctrlId == IDC_SETTINGS_INTERVALEDIT)
	{
		onIntervalEditKillFocus();
		return TRUE;
	}
	else {
		return FALSE;
	}
}



INT_PTR OptionsPageMore::handleNotify(const LPNMHDR header)
{
	if (header->code == PSN_APPLY)
	{
		bool fromApplyButton = !((LPPSHNOTIFY)header)->lParam;
		onApply(fromApplyButton);
		return TRUE;
	}
	else if (header->idFrom == IDC_SETTINGS_ABOUT) {
		if (header->code == NM_CLICK || header->code == NM_RETURN)
		{
			DialogBox(NULL, MAKEINTRESOURCE(IDD_ABOUT),
				GetParent(m_hwnd), aboutBoxDialogProc);
			return TRUE;
			WS_TABSTOP;
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}



void OptionsPageMore::onInitDialog()
{
	// Change control properties.
	Edit_LimitText(GetDlgItem(m_hwnd, IDC_SETTINGS_INTERVALEDIT), 5);
	SendDlgItemMessage(m_hwnd, IDC_SETTINGS_INTERVALUPDOWN, UDM_SETRANGE32,
		m_oldSettings.getMinInterval(), m_oldSettings.getMaxInterval());

	// Fill in verbosity list.
	// Length = length of longest text + 30%.
	TCHAR listEntry[64];
	HWND hList = GetDlgItem(m_hwnd, IDC_SETTINGS_VERBOSITYLIST);
	for (int i = IDS_VERBOSITY_DESC0; i <= IDS_VERBOSITY_DESC3; ++i)
	{
		LoadString(GetModuleHandle(NULL), i, listEntry, 64);
		ComboBox_AddString(hList, listEntry);
	}

	// Fill in defaults.
	SendDlgItemMessage(m_hwnd, IDC_SETTINGS_HOTKEYEDIT,
		HKM_SETHOTKEY, m_oldSettings.getHotkey(), 0);
	SetDlgItemInt(m_hwnd, IDC_SETTINGS_INTERVALEDIT,
		m_oldSettings.getInterval(), FALSE);
	ComboBox_SetCurSel(hList, (int) m_oldSettings.getVerbosity());
}



void OptionsPageMore::updateHasChanged()
{
	if (m_oldSettings == m_newSettings)
	{
		PropSheet_UnChanged(GetParent(m_hwnd), m_hwnd);
	}
	else {
		PropSheet_Changed(GetParent(m_hwnd), m_hwnd);
	}
}



void OptionsPageMore::onButtonClick(UINT ctrlId, HWND ctrlHandle)
{
	if (ctrlId == IDC_SETTINGS_AUTOSTARTBUTTON)
	{
		onAutoStartButtonClick();
	}
	else if (ctrlId == IDC_SETTINGS_CONNCUTBUTTON)
	{
		// AutoSave should exit when the shortcuts dialog tells it to.
		// But changes must be saved in the process.
		try {
			INT_PTR result = ShortcutsDialog::show(
				GetParent(m_hwnd), m_newSettings);
			if (result == IDABORT)
			{
				m_shouldAppExit = true;
				PropSheet_PressButton(GetParent(m_hwnd), PSBTN_OK);
			}
		}
		catch (AutoSaveException& exc) {
			exc.showMessageBox(m_hwnd, L"An error occured while trying to "
				L"open this dialog box.");
		}
		catch (std::exception& exc) {
			AutoSaveException::showMessageBox(m_hwnd,
				L"An error occured while trying to open this dialog box.", exc);
		}
	}
}



void OptionsPageMore::onEditChange(UINT ctrlId, HWND ctrlHandle)
{
	if (ctrlId == IDC_SETTINGS_HOTKEYEDIT)
	{
		LRESULT content = SendMessage(ctrlHandle, HKM_GETHOTKEY, 0, 0);
		m_newSettings.setHotkey(LOWORD(content));
	}
	else if (ctrlId == IDC_SETTINGS_INTERVALEDIT)
	{
		m_newSettings.setInterval(GetDlgItemInt(m_hwnd, ctrlId, NULL, FALSE));
	}
}



void OptionsPageMore::onComboBoxChange(UINT ctrlId, HWND ctrlHandle)
{
	m_newSettings.setVerbosity(ComboBox_GetCurSel(ctrlHandle));
}



// Refuse out-of-range numbers as early as possible.
void OptionsPageMore::onIntervalEditKillFocus()
{
	// When this function is called, setInterval has already been called
	// with an invalid argument. Its default behavior is to force the
	// invalid argument into the valid range. Thus, the following call
	// sets the edit control content to this forcedly valid value.
	SetDlgItemInt(m_hwnd, IDC_SETTINGS_INTERVALEDIT,
		m_newSettings.getInterval(), FALSE);
}



void OptionsPageMore::onAutoStartButtonClick()
{
	// String length + 30%
	TCHAR warning[400];
	LoadString(GetModuleHandle(NULL), IDS_AUTOSTARTWARNING, warning, 400);
	if (IDOK == MessageBox(GetParent(m_hwnd), warning, APP_NAME, MB_OKCANCEL))
	{
		try {
			AutostartShortcut::create();
			MessageBox(GetParent(m_hwnd),
				L"The shortcut has been successfully created.", APP_NAME, MB_OK);
		}
		catch (AutoSaveException& exc) {
			exc.showMessageBox(GetParent(m_hwnd), L"Couldn't create the shortcut.");
		}
	}
}



void OptionsPageMore::onApply(bool fromApplyButton)
{
	const int errorCapacity = 255;
	TCHAR errorMsg[errorCapacity];
	int errorSize = 0;

	if (LOBYTE(m_newSettings.getHotkey()) == 0)
	{
		errorSize = LoadString(GetModuleHandle(NULL),
			IDS_ERROR_NOHOTKEY, errorMsg, errorCapacity);
	}
	
	if (errorSize > 0)
	{
		MessageBox(GetParent(m_hwnd), errorMsg, APP_NAME, MB_ICONERROR);
		SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, PSNRET_INVALID);
	}
	else {
		m_newSettings = m_oldSettings;
		SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
	}
}



INT_PTR CALLBACK OptionsPageMore::aboutBoxDialogProc(
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		int iconSize = LOWORD(GetDialogBaseUnits()) * 24 / 4;
		auto appIcon = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_A),
			IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
		Static_SetIcon(GetDlgItem(hwnd, IDC_ABOUT_ICON), appIcon);
		SetDlgItemText(hwnd, IDC_ABOUT_VERSION, L"Version: " APP_VERSION);
		SetDlgItemText(hwnd, IDC_ABOUT_DATE, L"Compiled on: " APP_DATE);
		return TRUE;
	}
	if (uMsg == WM_NOTIFY)
	{
		auto pHdr = (const PNMLINK) lParam;
		if (pHdr->hdr.code == NM_CLICK || pHdr->hdr.code == NM_RETURN)
		{
			ShellExecute(NULL, L"open", pHdr->item.szUrl, NULL, NULL, SW_SHOW);
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else if (uMsg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		DestroyIcon(Static_GetIcon(GetDlgItem(hwnd, IDC_ABOUT_ICON), 0));
		EndDialog(hwnd, IDOK);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

