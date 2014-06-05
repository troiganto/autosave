// OptionsPageBase.h : Base class for pages of the options
// property sheet.
// 

#pragma once

class OptionsPageBase
{
public:
	OptionsPageBase();
	~OptionsPageBase();

	virtual HPROPSHEETPAGE create() = 0;

protected:
	static INT_PTR CALLBACK dialogProc(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

	HWND m_hwnd;
	static const int m_id = 0;

};

