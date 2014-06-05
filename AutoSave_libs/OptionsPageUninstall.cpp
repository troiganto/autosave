#include "stdafx.h"
#include "OptionsPageUninstall.h"


OptionsPageUninstall::OptionsPageUninstall()
	: m_shouldAppExit(false), shouldAppExit(m_shouldAppExit)
{
}


OptionsPageUninstall::~OptionsPageUninstall()
{
}



HPROPSHEETPAGE OptionsPageUninstall::create()
{
	PROPSHEETPAGE psp = { 0 };
	psp.dwSize = sizeof(psp);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = GetModuleHandle(NULL);
	psp.pszTemplate = MAKEINTRESOURCE(IDD_SETTINGS_UNINSTALL);
	psp.pfnDlgProc = dialogProc;
	psp.lParam = (LPARAM) this;
	return CreatePropertySheetPage(&psp);
}



INT_PTR OptionsPageUninstall::handleMessage(
	UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_INITDIALOG)
	{
		return TRUE;
	}
	else if (message == WM_COMMAND)
	{
		return handleCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
	}
	else {
		return FALSE;
	}
}



INT_PTR OptionsPageUninstall::handleCommand(UINT code, UINT ctrlId, HWND ctrlHandle)
{
	if (code == BN_CLICKED)
	{
		switch (ctrlId)
		{
		case IDC_UNINSTALL_ALL:
			callUninstaller(Uninstaller::ALL);
			return TRUE;
		case IDC_UNINSTALL_AUTOSTART:
			callUninstaller(Uninstaller::AUTOSTART);
			return TRUE;
		case IDC_UNINSTALL_CONNCUT:
			callUninstaller(Uninstaller::SHORTCUTS);
			return TRUE;
		case IDC_UNINSTALL_REGISTRY:
			callUninstaller(Uninstaller::REGISTRY);
			return TRUE;
		default:
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}

void OptionsPageUninstall::callUninstaller(Uninstaller::Uninstall mode)
{
	try {
		// If the uninstaller deletes AutoSave's registry entry,
		// AutoSave should exit wihout further question. (i.e. cancel changes)
		INT_PTR uninstallerResult = Uninstaller::start(GetParent(m_hwnd), mode);
		if (uninstallerResult == ID_PSRESTARTWINDOWS)
		{
			m_shouldAppExit = true;
			PropSheet_PressButton(GetParent(m_hwnd), PSBTN_CANCEL);
		}
	}
	catch (AutoSaveException& exc) {
		exc.showMessageBox(m_hwnd, L"An error occured while trying to "
			L"open the uninstall wizard.");
	}
	catch (std::exception& exc) {
		AutoSaveException::showMessageBox(m_hwnd,
			L"An error occured while trying to open the uninstall wizard.", exc);
	}
}