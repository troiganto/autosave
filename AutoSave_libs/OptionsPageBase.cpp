#include "stdafx.h"
#include "OptionsPageBase.h"


OptionsPageBase::OptionsPageBase()
{
}



OptionsPageBase::~OptionsPageBase()
{
}



INT_PTR CALLBACK OptionsPageBase::dialogProc(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	OptionsPageBase* pThis = NULL;

	if (uMsg == WM_INITDIALOG)
	{
		const auto pPsp = (PROPSHEETPAGE*) lParam;
		pThis = (OptionsPageBase*) pPsp->lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)pThis);
		// Set the m_hwnd member as early as possible.
		pThis->m_hwnd = hwndDlg;
	}
	else {
		pThis = (OptionsPageBase*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	}

	if (pThis)
	{
		return pThis->handleMessage(uMsg, wParam, lParam);
	}
	else {
		return FALSE;
	}
}