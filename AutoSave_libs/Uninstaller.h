// Uninstaller.h : class for the wizard that helps people remove
// components of AutoSave.
// Must absorb all exceptions.

#pragma once

#include "stdafx.h"
#include "IconBitmap.h"
#include "RegistryAccess.h"
#include "AutostartShortcut.h"
#include "OptionsPageBase.h"
#include "ShortcutsDisconnector.h"
#include "UninstallerShortcutsListbox.h"
#include "..\AutoSave\\Resource.h"

class Uninstaller
{
public:
	enum Uninstall {
		AUTOSTART,
		SHORTCUTS,
		REGISTRY,
		ALL
	};

	static INT_PTR start(HWND hwndParent, Uninstall mode);

protected:
	Uninstaller() : m_lastException(S_OK) {}

	typedef void(Uninstaller::*UninstallAction)();

	void createPages(Uninstall mode, HPROPSHEETPAGE hPages[], UINT* pNPages);
	UINT createComplexPages(PROPSHEETPAGE* ppsp, HPROPSHEETPAGE hPages[]);
	UINT createSimplePages(PROPSHEETPAGE* ppsp,
		Uninstall mode, HPROPSHEETPAGE hPages[]);

	static INT_PTR CALLBACK handleMessages(
		HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR handleIntroPage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR handleAutoStartPage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR handleShortcutsPage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR handleRegistryPage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR handleErrorPage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR handleOutroPage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// These four functions may throw OleException.
	void removeAutoStartShortcut();
	void removeRegistryEntries();
	void removeConnectedShortcuts();
	void convertConnectedShortcuts();

	INT_PTR confirmWizardCancel(HWND hwnd) const;
	INT_PTR continueWizard(HWND hwnd, UninstallAction doAction);

private:
	wstring getErrorPageText();
	void refreshShortcutsPage(HWND hwnd) const;

	static const int maxPagesCount = 6;
	UninstallerShortcutsListbox m_usListbox;

	bool m_isComplexWizard;
	AutoSaveException m_lastException;
};
