#include "stdafx.h"
#include "OptionsPageTarget.h"


OptionsPageTarget::OptionsPageTarget(Matcher* pFilter)
	: m_oldMatcher(*pFilter), // This is a reference,
	  m_newMatcher(m_oldMatcher) // this is a copy.
{
	loadEmptyString(IDS_TARGET_NOWINDOWS);
}



OptionsPageTarget::~OptionsPageTarget()
{
}



HPROPSHEETPAGE OptionsPageTarget::create()
{
	PROPSHEETPAGE psp = { 0 };
	psp.dwSize = sizeof(psp);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = GetModuleHandle(NULL);
	psp.pszTemplate = MAKEINTRESOURCE(IDD_SETTINGS_TARGET);
	psp.pfnDlgProc = dialogProc;
	psp.lParam = (LPARAM) this;
	return CreatePropertySheetPage(&psp);
}



INT_PTR OptionsPageTarget::handleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_INITDIALOG)
	{
		onInitDialog();
		return TRUE;
	}
	else if (message == WM_NOTIFY)
	{
		return handleNotify((NMHDR*) lParam);
	}
	else if (message == WM_COMMAND)
	{
		return handleCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
	}
	else if (message == WM_TIMER)
	{
		onTimer(wParam);
		return TRUE;
	}
	else {
		return FALSE;
	}
}



INT_PTR OptionsPageTarget::handleCommand(UINT code, UINT ctrlId, HWND ctrlHandle)
{
	if (code == BN_CLICKED)
	{
		onButtonClick(ctrlId, ctrlHandle);
		updateHasChanged();
		return TRUE;
	}
	else if (code == EN_CHANGE)
	{
		onEditChange(ctrlId, ctrlHandle);
		updateHasChanged();
		return TRUE;
	}
	else {
		return FALSE;
	}
}



INT_PTR OptionsPageTarget::handleNotify(const LPNMHDR pHeader)
{
	if (pHeader->code == PSN_SETACTIVE)
	{
		onSetActive();
		return TRUE;
	}
	else if (pHeader->code == PSN_KILLACTIVE)
	{
		onKillActive();
		return TRUE;
	}
	else if (pHeader->code == PSN_APPLY)
	{
		bool fromApplyButton = !((LPPSHNOTIFY)pHeader)->lParam;
		onApply(fromApplyButton);
		return TRUE;
	}
	else if (pHeader->idFrom == IDC_SETTINGS_REGEXHELP &&
		(pHeader->code == NM_CLICK || pHeader->code == NM_RETURN))
	{
		onLinkClicked(((const PNMLINK) pHeader)->item.szUrl);
		return TRUE;
	}
	else {
		return FALSE;
	}
}



// Set controls on initial values.
void OptionsPageTarget::onInitDialog()
{
	SetDlgItemText(m_hwnd, IDC_SETTINGS_FILTEREDIT,
		m_oldMatcher.getFilter().data());

	CheckDlgButton(m_hwnd, IDC_SETTINGS_ISREGEXCHECK,
		m_oldMatcher.isRegex() ? BST_CHECKED : BST_UNCHECKED);

	HFONT hFont = GetStockFont(
		m_oldMatcher.isRegex() ? ANSI_FIXED_FONT : ANSI_VAR_FONT);
	SendDlgItemMessage(m_hwnd, IDC_SETTINGS_FILTEREDIT,
		WM_SETFONT, (WPARAM) hFont, TRUE);

	SetWindowSubclass(GetDlgItem(m_hwnd, IDC_SETTINGS_WINDOWLIST),
		GdiUtils::subclassProcForListboxWithEmptyText, 0,
		(DWORD_PTR) m_windowListEmptyText);
}



void OptionsPageTarget::updateHasChanged()
{
	if (m_newMatcher == m_oldMatcher)
	{
		PropSheet_UnChanged(GetParent(m_hwnd), m_hwnd);
	}
	else {
		PropSheet_Changed(GetParent(m_hwnd), m_hwnd);
	}
}



// Switch from phrase to regex and vice versa when
// the checkbox has been clicked.
void OptionsPageTarget::onButtonClick(UINT ctrlId, HWND ctrlHandle)
{
	m_newMatcher.useRegex(Button_GetCheck(ctrlHandle) == BST_CHECKED);

	wstring newText = m_newMatcher.getFilter();
	HFONT newFont = GetStockFont(
		m_newMatcher.isRegex() ? ANSI_FIXED_FONT : ANSI_VAR_FONT
		);

	SetDlgItemText(m_hwnd, IDC_SETTINGS_FILTEREDIT, newText.data());
	SendDlgItemMessage(m_hwnd, IDC_SETTINGS_FILTEREDIT,
		WM_SETFONT, (WPARAM) newFont, TRUE);
}

// Update the config object when the filter text
// has changed.
void OptionsPageTarget::onEditChange(UINT ctrlId, HWND ctrlHandle)
{
	wstring filter = Matcher::getWindowText(
		GetDlgItem(m_hwnd, IDC_SETTINGS_FILTEREDIT));

	if (m_newMatcher.isRegex())
	{
		m_newMatcher.setRegex(filter);
	}
	else {
		m_newMatcher.setPhrase(filter);
	}
}



void OptionsPageTarget::onLinkClicked(LPCWSTR linkUrl)
{
	ShellExecute(NULL, L"open", linkUrl, NULL, NULL, SW_SHOW);
}



void OptionsPageTarget::onSetActive()
{
	updateWindowList();
	updateCurrentWindowCaptionEdit();
	SetTimer(m_hwnd, listTimerId, 1000, NULL);
	SetTimer(m_hwnd, captionTimerId, 100, NULL);
}

void OptionsPageTarget::onKillActive()
{
	KillTimer(m_hwnd, listTimerId);
	KillTimer(m_hwnd, captionTimerId);
}

// Verify input and save settings.
void OptionsPageTarget::onApply(bool fromApplyButton)
{
	const int errorCapacity = 255;
	TCHAR errorMsg[errorCapacity];
	int errorSize = 0;

	if (m_newMatcher.isEmpty())
	{
		errorSize = LoadString(GetModuleHandle(NULL),
			IDS_ERROR_NOTARGET, errorMsg, errorCapacity);
	}
	else if (m_newMatcher.isRegex() && m_newMatcher.isRegexBad())
	{
		errorSize = LoadString(GetModuleHandle(NULL),
			IDS_ERROR_BADREGEX, errorMsg, errorCapacity);
	}

	if (errorSize > 0)
	{
		MessageBox(GetParent(m_hwnd), errorMsg, APP_NAME, MB_ICONERROR);
		SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, PSNRET_INVALID);
	}
	else {
		m_oldMatcher.setFilter(m_newMatcher);
		SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
	}
}



void OptionsPageTarget::onTimer(UINT_PTR timerId)
{
	if (timerId == captionTimerId)
	{
		updateCurrentWindowCaptionEdit();
	}
	else {
		updateWindowList();
	}
}



void OptionsPageTarget::updateCurrentWindowCaptionEdit()
{
	// Naive approach would fuck with the text selection.
	static wstring currentText;
	
	POINT point;
	GetCursorPos(&point);

	HWND window = ChildWindowFromPointEx(
		GetDesktopWindow(), point, CWP_SKIPINVISIBLE);
	/* Maybe the following is a good addition?
	if (window == GetParent(m_hwnd))
		return;
	*/
	
	wstring newText = Matcher::getWindowText(window);
	if (newText != currentText)
	{
		currentText = newText;
		SetDlgItemText(m_hwnd, IDC_SETTINGS_CURWINEDIT,
			currentText.data());
	}
}



// Catch bad input before iterating over all windows.
void OptionsPageTarget::updateWindowList()
{
	HWND hList = GetDlgItem(m_hwnd, IDC_SETTINGS_WINDOWLIST);
	SendMessage(hList, WM_SETREDRAW, FALSE, 0);
	ListBox_ResetContent(hList);

	if (m_newMatcher.isEmpty())
	{
		loadEmptyString(IDS_TARGET_EMPTY);
	}
	else if (m_newMatcher.isRegex() && m_newMatcher.isRegexBad())
	{
		loadEmptyString(IDS_TARGET_BADREGEX);
	}
	else {
		fillWindowList(hList, m_newMatcher);
		if (ListBox_GetCount(hList) == 0)
			loadEmptyString(IDS_TARGET_NOWINDOWS);
	}
	SendMessage(hList, WM_SETREDRAW, TRUE, 0);
	RedrawWindow(hList, NULL, NULL,
		RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}



void OptionsPageTarget::fillWindowList(HWND hList, const Matcher& filter)
{
	RECT listRect;
	GetClientRect(hList, &listRect);
	const int itemHeight = ListBox_GetItemHeight(hList, 0);

	fillWindowListEnumProcParameters args = {
		hList, filter, listRect.bottom / itemHeight
	};
	EnumWindows(fillWindowListEnumProc, (LPARAM)&args);
}



BOOL CALLBACK OptionsPageTarget::fillWindowListEnumProc(HWND hwnd, LPARAM lParam)
{
	// Ignore owned and invisible windows.
	bool isUnownedAndVisible = GetParent(hwnd) == 0 && IsWindowVisible(hwnd);
	if (!isUnownedAndVisible)
		return TRUE;

	// Ignore windows without caption.
	auto pArgs = (fillWindowListEnumProcParameters*)lParam;
	wstring caption = Matcher::getWindowText(hwnd);
	if (caption.empty())
		return TRUE;

	// Ignore windows that don't match.
	if (!pArgs->filter.match(caption))
		return TRUE;

	// Matching windows reduce the counter.
	--pArgs->remainingWindows;
	ListBox_AddString(pArgs->hList,
		(pArgs->remainingWindows != 0) ? caption.data() : L"[...]");
	return pArgs->remainingWindows > 0;
}


