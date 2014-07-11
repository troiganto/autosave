// OptionsPageTarget.h : This page handles which windows should
// be targeted by AutoSave.
// No exceptions are thrown inside this class.

#pragma once

#include "stdafx.h"
#include "GdiUtils.h"
#include "OptionsPageBase.h"
#include "Matcher.h"
#include "..\AutoSave\\Resource.h"

class OptionsPageTarget : public OptionsPageBase
{
public:
	OptionsPageTarget(Matcher* pFilter);
	~OptionsPageTarget();

	HPROPSHEETPAGE create();

protected:
	INT_PTR handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR handleNotify(LPNMHDR header);
	INT_PTR handleCommand(UINT code, UINT ctrlId, HWND ctrlHandle);

	// Event handlers
	void onInitDialog();
	void updateHasChanged();

	void onButtonClick(UINT ctrlId, HWND ctrlHandle);
	void onEditChange(UINT ctrlId, HWND ctrlHandle);
	void onLinkClicked(LPCWSTR linkUrl);

	void onSetActive();
	void onKillActive();
	void onApply(bool fromApplyButton);

	void onTimer(UINT_PTR timerId);

private:
	void updateCurrentWindowCaptionEdit();

	// Filling the window list
	void updateWindowList();
	static void fillWindowList(HWND hList, const Matcher& filter);
	static BOOL CALLBACK fillWindowListEnumProc(HWND hwnd, LPARAM lParam);
	struct fillWindowListEnumProcParameters {
		const HWND hList;
		const Matcher& filter;
		UINT remainingWindows;
	};


	Matcher& m_oldMatcher;
	Matcher m_newMatcher;

	const UINT_PTR listTimerId = 1;
	const UINT_PTR captionTimerId = 2;
	const static int m_emptyTextMaxSize = 128;
	wchar_t m_windowListEmptyText[m_emptyTextMaxSize];
	inline void loadEmptyString(UINT stringId) {
		LoadString(GetModuleHandle(NULL), stringId,
			m_windowListEmptyText, m_emptyTextMaxSize);
	}

};

