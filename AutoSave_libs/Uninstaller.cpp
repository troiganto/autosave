#include "stdafx.h"
#include "Uninstaller.h"


// Returns ID_PSRESTARTWINDOWS if AutoSave should exit
// after calling this function.
INT_PTR Uninstaller::start(HWND hwndParent, Uninstall mode)
{
	Uninstaller unst;
	IconBitmap ib(MAKEINTRESOURCE(IDI_A), 49);

	HPROPSHEETPAGE hPages[maxPagesCount];
	UINT nPages = 0;
	unst.createPages(mode, hPages, &nPages);
	
	PROPSHEETHEADER psh = { 0 };
	psh.dwSize = sizeof(psh);
	psh.dwFlags = PSH_USEHICON | PSH_NOCONTEXTHELP | PSH_WIZARD97 |
		PSH_HEADER | PSH_USEHBMHEADER;
	psh.hbmHeader = ib.hBitmap;
	psh.hwndParent = hwndParent;
	psh.hInstance = GetModuleHandle(NULL);
	psh.hIcon = LoadAppIcon(IDI_A);
	psh.nPages = nPages;
	psh.nStartPage = 0;
	psh.phpage = hPages;

	INT_PTR result = PropertySheet(&psh);
	DeleteBitmap(psh.hbmHeader);
	return result;
}



void Uninstaller::createPages(Uninstall mode, HPROPSHEETPAGE hPages[], UINT* pNPages)
{
	PROPSHEETPAGE psp = { 0 };
	psp.dwSize = sizeof(psp);
	psp.dwFlags = PSP_USEHEADERTITLE | PSP_USETITLE | PSP_USEHEADERSUBTITLE;
	psp.pszHeaderTitle = L"Uninstall " SHORT_APP_NAME L" Components";
	psp.pszTitle = psp.pszHeaderTitle;
	psp.hInstance = GetModuleHandle(NULL);
	psp.pfnDlgProc = handleMessages;
	psp.lParam = (LPARAM) this;

	if (mode == Uninstall::ALL)
	{
		*pNPages = createComplexPages(&psp, hPages);
	}
	else {
		*pNPages = createSimplePages(&psp, mode, hPages);
	}
}



UINT Uninstaller::createComplexPages(PROPSHEETPAGE* ppsp, HPROPSHEETPAGE hPages[])
{
	m_isComplexWizard = true;
	ppsp->pszTemplate = MAKEINTRESOURCE(IDD_UNINSTALL_INTRO);
	ppsp->pszHeaderSubTitle = L"Uninstall " SHORT_APP_NAME;
	hPages[0] = CreatePropertySheetPage(ppsp);
	ppsp->pszTemplate = MAKEINTRESOURCE(IDD_UNINSTALL_AUTOSTART);
	ppsp->pszHeaderSubTitle = L"Remove Auto-start shortcut";
	hPages[1] = CreatePropertySheetPage(ppsp);
	ppsp->pszTemplate = MAKEINTRESOURCE(IDD_UNINSTALL_SHORTCUTS);
	ppsp->pszHeaderSubTitle = L"Revert Connected Shortcuts";
	hPages[2] = CreatePropertySheetPage(ppsp);
	ppsp->pszTemplate = MAKEINTRESOURCE(IDD_UNINSTALL_REGISTRY);
	ppsp->pszHeaderSubTitle = L"Delete saved options";
	hPages[3] = CreatePropertySheetPage(ppsp);
	ppsp->pszTemplate = MAKEINTRESOURCE(IDD_UNINSTALL_ERROR);
	ppsp->pszHeaderSubTitle = L"Finish wizard";
	hPages[4] = CreatePropertySheetPage(ppsp);
	ppsp->pszTemplate = MAKEINTRESOURCE(IDD_UNINSTALL_OUTRO);
	hPages[5] = CreatePropertySheetPage(ppsp);
	return 6;
}



UINT Uninstaller::createSimplePages(
	PROPSHEETPAGE* ppsp, Uninstall mode, HPROPSHEETPAGE hPages[])
{
	m_isComplexWizard = false;
	switch (mode)
	{
	case Uninstall::AUTOSTART:
		ppsp->pszTemplate = MAKEINTRESOURCE(IDD_UNINSTALL_AUTOSTART);
		ppsp->pszHeaderSubTitle = L"Remove Auto-start shortcut";
		break;
	case Uninstall::SHORTCUTS:
		ppsp->pszTemplate = MAKEINTRESOURCE(IDD_UNINSTALL_SHORTCUTS);
		ppsp->pszHeaderSubTitle = L"Revert Connected Shortcuts";
		break;
	case Uninstall::REGISTRY:
		ppsp->pszTemplate = MAKEINTRESOURCE(IDD_UNINSTALL_REGISTRY);
		ppsp->pszHeaderSubTitle = L"Delete saved options";
		break;
	default:
		return 0;
	}
	hPages[0] = CreatePropertySheetPage(ppsp);
	ppsp->pszTemplate = MAKEINTRESOURCE(IDD_UNINSTALL_ERROR);
	ppsp->pszHeaderSubTitle = L"Finish wizard";
	hPages[1] = CreatePropertySheetPage(ppsp);
	return 2;
}



INT_PTR CALLBACK Uninstaller::handleMessages(
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Uninstaller* pThis = NULL;

	if (uMsg == WM_INITDIALOG)
	{
		const auto pPsp = (PROPSHEETPAGE*)lParam;
		pThis = (Uninstaller*) pPsp->lParam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) pThis);
	}
	else {
		pThis = (Uninstaller*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (pThis)
	{
		int id = 0;
		if (uMsg == WM_INITDIALOG)
		{
			id = (WORD) ((PROPSHEETPAGE*)lParam)->pszTemplate;
		}
		else {
			int index = PropSheet_HwndToIndex(GetParent(hwnd), hwnd);
			id = (ULONG) PropSheet_IndexToId(GetParent(hwnd), index);
		}
		switch (id)
		{
		case IDD_UNINSTALL_INTRO:
			return pThis->handleIntroPage(hwnd, uMsg, wParam, lParam);
		case IDD_UNINSTALL_AUTOSTART:
			return pThis->handleAutoStartPage(hwnd, uMsg, wParam, lParam);
		case IDD_UNINSTALL_SHORTCUTS:
			return pThis->handleShortcutsPage(hwnd, uMsg, wParam, lParam);
		case IDD_UNINSTALL_REGISTRY:
			return pThis->handleRegistryPage(hwnd, uMsg, wParam, lParam);
		case IDD_UNINSTALL_ERROR:
			return pThis->handleErrorPage(hwnd, uMsg, wParam, lParam);
		case IDD_UNINSTALL_OUTRO:
			return pThis->handleOutroPage(hwnd, uMsg, wParam, lParam);
		default:
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}



INT_PTR Uninstaller::handleIntroPage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NOTIFY)
	{
		const PSHNOTIFY& header = *(LPPSHNOTIFY) lParam;
		if (header.hdr.code == PSN_SETACTIVE)
		{
			PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_NEXT);
			return 0;
		}
		else {
			return FALSE;
		}
	}
	else {
		return uMsg == WM_INITDIALOG ? TRUE : FALSE;
	}
}



INT_PTR Uninstaller::handleAutoStartPage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NOTIFY)
	{
		const PSHNOTIFY& header = *(LPPSHNOTIFY)lParam;
		if (header.hdr.code == PSN_SETACTIVE)
		{
			PropSheet_SetWizButtons(GetParent(hwnd),
				PSWIZB_NEXT | (m_isComplexWizard ? PSWIZB_BACK : 0));
			return 0;
		}
		else if (header.hdr.code == PSN_WIZNEXT)
		{
			return continueWizard(hwnd, &Uninstaller::removeAutoStartShortcut);
		}
		else {
			return FALSE;
		}
	}
	else {
		return uMsg == WM_INITDIALOG ? TRUE : FALSE;
	}
}



INT_PTR Uninstaller::handleShortcutsPage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NOTIFY)
	{
		const PSHNOTIFY& header = *(LPPSHNOTIFY)lParam;
		switch (header.hdr.code)
		{
		case PSN_SETACTIVE: {
			PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_NEXT);
			CheckDlgButton(hwnd, IDC_UNINSTALL_CONVERT, BST_CHECKED);
			m_usListbox.connect(hwnd, GetDlgItem(hwnd, IDC_UNINSTALL_FILELIST));
			refreshShortcutsPage(hwnd);
			return 0;
			}
		case PSN_WIZNEXT: {
			// Here comes the actual operation.
			bool doConvertDontDelete = Button_GetCheck(
				GetDlgItem(hwnd, IDC_UNINSTALL_CONVERT)) == BST_CHECKED;
			return continueWizard(hwnd, doConvertDontDelete ?
				&Uninstaller::convertConnectedShortcuts :
				&Uninstaller::removeConnectedShortcuts);
			}
		case PSN_QUERYCANCEL: return confirmWizardCancel(hwnd);
		default: return FALSE;
		}
	}
	else if (uMsg == m_usListbox.LB_REFRESH)
	{
		refreshShortcutsPage(hwnd);
		return 0;
	}
	else if (uMsg == m_usListbox.LB_EXCEPTIONTHROWN)
	{
		// Acts like a catch clause surrounding the whole if construct,
		// but also catches exceptions in m_usListbox.Drop.
		AutoSaveException exc(m_usListbox.lastException);
		exc.showMessageBox(hwnd, L"An error occured while trying to "
			L"add connected shortcuts to this list.");
		return 0;
	}
	else {
		return uMsg == WM_INITDIALOG ? TRUE : FALSE;
	}
}



INT_PTR Uninstaller::handleRegistryPage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NOTIFY)
	{
		const PSHNOTIFY& header = *(LPPSHNOTIFY)lParam;
		if (header.hdr.code == PSN_SETACTIVE)
		{
			PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_NEXT);
			return 0;
		}
		else if (header.hdr.code == PSN_WIZNEXT)
		{
			return continueWizard(hwnd, &Uninstaller::removeRegistryEntries);
		}
		else if (header.hdr.code == PSN_QUERYCANCEL)
		{
			return confirmWizardCancel(hwnd);
		}
		else {
			return FALSE;
		}
	}
	else {
		return uMsg == WM_INITDIALOG ? TRUE : FALSE;
	}
}



INT_PTR Uninstaller::handleErrorPage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NOTIFY)
	{
		const PSHNOTIFY& header = *(LPPSHNOTIFY)lParam;
		if (header.hdr.code == PSN_SETACTIVE)
		{
			PropSheet_CancelToClose(GetParent(hwnd));
			if (m_isComplexWizard && SUCCEEDED(m_lastException.hResult()))
			{
				// Complex wizard skips this page on success.
				SetWindowLongPtr(hwnd, DWLP_MSGRESULT, -1);
				return TRUE;
			}
			else {
				PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_FINISH);
				// Set text based on success.
				SetDlgItemText(hwnd, IDC_UNINSTALL_ERRORTEXT,
					getErrorPageText().data());
				return FALSE;
			}
		}
		else {
			return FALSE;
		}
	}
	else {
		return uMsg == WM_INITDIALOG ? TRUE : FALSE;
	}
}



INT_PTR Uninstaller::handleOutroPage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NOTIFY)
	{
		const PSHNOTIFY& header = *(LPPSHNOTIFY)lParam;
		if (header.hdr.code == PSN_SETACTIVE)
		{
			PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_FINISH);
			return 0;
		}
		else {
			return FALSE;
		}
	}
	else {
		return uMsg == WM_INITDIALOG ? TRUE : FALSE;
	}
}



void Uninstaller::removeAutoStartShortcut()
{
	AutostartShortcut::remove();
}



void Uninstaller::removeRegistryEntries()
{
	RegistryAccess ra;
	ra.access(DEFAULT_REGISTRY_KEY);
	ra.purge();
}



void Uninstaller::removeConnectedShortcuts()
{
	ShortcutsDisconnector::removeShortcuts(m_usListbox.files);
}



void Uninstaller::convertConnectedShortcuts()
{
	ShortcutsDisconnector::disconnectShortcuts(m_usListbox.files);
}



INT_PTR Uninstaller::confirmWizardCancel(HWND hwnd) const
{
	if (!m_isComplexWizard)
		return FALSE;

	if (IDYES == MessageBox(hwnd, L"The removal process has not been completed yet. "
		L"Are you sure you want to exit?", APP_NAME, MB_YESNO | MB_ICONQUESTION))
	{
		return FALSE;
	}
	else {
		SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
		return TRUE;
	}
}



// Executes the function passed as the doAction argument.
// If this action fails, the wizard navigates to an error page.
// Otherwise, it just shows the next wizard page.
INT_PTR Uninstaller::continueWizard(HWND hwnd, UninstallAction doAction)
{
	try {
		(this->*doAction)();
		// Close AutoSave only if we're dealing with the registry deletion.
		if (doAction == &Uninstaller::removeRegistryEntries)
			PropSheet_RestartWindows(GetParent(hwnd));
		return FALSE;
	}
	catch (AutoSaveException& exc) {
		m_lastException = exc;
		SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_UNINSTALL_ERROR);
		return TRUE;
	}
	catch (std::exception&) {
		m_lastException = E_FAIL;
		SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_UNINSTALL_ERROR);
		return TRUE;
	}
}



wstring Uninstaller::getErrorPageText()
{
	if (SUCCEEDED(m_lastException.hResult()))
	{
		return L"The action has been completed successfully.\n"
			L"You may close this wizard now.";
	}
	else {
		MessageBeep(MB_ICONERROR);
		wstring output = L"An error occured and the wizard cannot be completed."
			L"This is the error message returned by the system:\n\n";
		output.append(m_lastException.wcwhat());
		return output;
	}
}



void Uninstaller::refreshShortcutsPage(HWND hwnd) const
{
	BOOL st = !m_usListbox.files.empty();
	EnableWindow(GetDlgItem(hwnd, IDC_UNINSTALL_CONVERT), st);
	EnableWindow(GetDlgItem(hwnd, IDC_UNINSTALL_DELETE), st);
}



