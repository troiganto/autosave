#include "stdafx.h"
#include "GdiUtils.h"



namespace {
	void presetDC(HDC hdc);
	int getTextHeight(HDC hdc, const wstring& text);
	void setMargins(LPRECT pClientRect, int textHeight);
	int drawText(HDC hdc, const wstring& text, LPRECT pRect, bool isVisible);
}



void GdiUtils::drawCenteredText(HWND hwnd, const wstring& emptyText)
{
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	PAINTSTRUCT ps;
	BeginPaint(hwnd, &ps);
	presetDC(ps.hdc);
	if (ps.fErase)
		FillRect(ps.hdc, &clientRect, GetSysColorBrush(COLOR_WINDOW));

	const int textHeight = drawText(ps.hdc, emptyText, &clientRect, false);
	setMargins(&clientRect, textHeight);
	drawText(ps.hdc, emptyText, &clientRect, true);
	EndPaint(hwnd, &ps);
}



LRESULT CALLBACK GdiUtils::subclassProcForListboxWithEmptyText(HWND hwnd,
	UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR scid, DWORD_PTR refData)
{
	if (uMsg == WM_PAINT && ListBox_GetCount(hwnd) == 0)
	{
		drawCenteredText(hwnd, (LPCWSTR)refData);
		return 0;
	}
	else {
		return DefSubclassProc(hwnd, uMsg, wParam, lParam);
	}
}



LRESULT CALLBACK GdiUtils::subclassProcForListviewWithEmptyText(HWND hwnd,
	UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR scid, DWORD_PTR refData)
{
	if (uMsg == WM_PAINT && ListView_GetItemCount(hwnd) == 0)
	{
		drawCenteredText(hwnd, (LPCWSTR)refData);
		return 0;
	}
	else {
		return DefSubclassProc(hwnd, uMsg, wParam, lParam);
	}
}



namespace {
	void presetDC(HDC hdc)
	{
		SelectFont(hdc, GetStockFont(ANSI_VAR_FONT));
		SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
		SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
	}

	void setMargins(LPRECT pClientRect, int textHeight)
	{
		// Yes, this is a constant pixel width in InflateRect.
		// I really, sincerely expect this to never become a problem.
		int verticalMargin = (pClientRect->bottom - textHeight) / 2;
		InflateRect(pClientRect, -6, -verticalMargin);
	}

	int drawText(HDC hdc, const wstring& text, LPRECT pRect, bool isVisible)
	{
		return DrawText(
			hdc, text.data(), (ULONG) text.size(), pRect,
			DT_TOP | DT_CENTER | DT_WORDBREAK |
			(isVisible ? DT_NOPREFIX : DT_PREFIXONLY));
	}
}



