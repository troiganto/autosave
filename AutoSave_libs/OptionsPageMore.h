// OptionsPageMore.h : This page handles other settings and
// gives access to various tools.
// Must absorb all exceptions.

#pragma once

#include "stdafx.h"
#include "OptionsPageBase.h"
#include "MiscSettings.h"
#include "AutostartShortcut.h"
#include "ShortcutsDialog.h"
#include "..\AutoSave\\Resource.h"

class OptionsPageMore : public OptionsPageBase
{
public:
	OptionsPageMore(MiscSettings* pSettings);
	~OptionsPageMore();

	HPROPSHEETPAGE create();
	const bool& shouldAppExit;

protected:
	INT_PTR handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR handleNotify(LPNMHDR header);
	INT_PTR handleCommand(UINT code, UINT ctrlId, HWND ctrlHandle);

	// Event handlers
	void onInitDialog();
	void updateHasChanged();

	void onButtonClick(UINT ctrlId, HWND ctrlHandle);
	void onEditChange(UINT ctrlId, HWND ctrlHandle);
	void onComboBoxChange(UINT ctrlId, HWND ctrlHandle);
	void onIntervalEditKillFocus();

	void onAutoStartButtonClick();

	void onApply(bool fromApplyButton);

	static INT_PTR CALLBACK aboutBoxDialogProc(
		HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	MiscSettings& m_oldSettings;
	MiscSettings m_newSettings;

	bool m_shouldAppExit;
};

