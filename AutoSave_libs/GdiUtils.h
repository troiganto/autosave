// GdiUtils.h : Provides function related to drawing text inside controls.
// Functions defined here never throw.

#pragma once

#include "stdafx.h"
#include <wingdi.h>

using std::wstring;

namespace GdiUtils {
	// You have to check yourself whether the control is actually empty.
	void drawCenteredText(HWND hwnd, const wstring& emptyText);

	// The refData argument must be of type LPCWSTR, pointing to the
	// text to be displayed.
	LRESULT CALLBACK subclassProcForListboxWithEmptyText(HWND hwnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam, UINT_PTR scid, DWORD_PTR refData);
	LRESULT CALLBACK subclassProcForListviewWithEmptyText(HWND hwnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam, UINT_PTR scid, DWORD_PTR refData);

}
