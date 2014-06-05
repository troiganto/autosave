// UninstallerShortcutsListTooltip.h : Wraps a tooltip that appears
// if the user dropped files in the listbox, but none of them was valid.
// Doesn't throw, just fails silently.

#pragma once

#include "stdafx.h"
#include "..\AutoSave\Resource.h"

class UninstallerShortcutsListTooltip
{
public:
	UninstallerShortcutsListTooltip();
	~UninstallerShortcutsListTooltip();

	void init(HWND hwndDialog, HWND hwndTool);
	void show(const POINTL& pt);
	void hide();

protected:
	HWND m_tooltip;
	TOOLINFO m_tooltipInfo;
	static const UINT showDelay = 1500;
	static const int timerId = 0;
};
