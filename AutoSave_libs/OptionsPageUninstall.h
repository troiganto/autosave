// OptionsPageUnisntall.h : This page lets the user revert some or all
// changes AutoSave has applied on their computer.
// Basically only contains buttons.
// No exception occur on this level.

#pragma once

#include "stdafx.h"
#include "OptionsPageBase.h"
#include "Uninstaller.h"
#include "..\AutoSave\\Resource.h"

class OptionsPageUninstall : OptionsPageBase
{
public:
	OptionsPageUninstall();
	~OptionsPageUninstall();

	HPROPSHEETPAGE create();

	const bool& shouldAppExit;

private:
	INT_PTR handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR handleCommand(UINT code, UINT ctrlId, HWND ctrlHandle);

	// Event handlers
	void callUninstaller(Uninstaller::Uninstall mode);

	// Bool that is set to true if AutoSave should exit ASAP.
	bool m_shouldAppExit;
};

