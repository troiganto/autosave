// OptionsWindow.h : Supplies a property sheet to manipulate
// a given Configuration object. Wraps around the OptionsPage* objects.
// No exceptions occur on this level.

#pragma once

#include "stdafx.h"
#include "Configuration.h"
#include "OptionsPageTarget.h"
#include "OptionsPageMore.h"
#include "OptionsPageUninstall.h"
#include "..\AutoSave\\Resource.h"

using std::unordered_map;

class OptionsWindow {

public:
	~OptionsWindow();

	enum PageNumber {
		TargetPage,
		MoreOptionsPage,
		UninstallPage
	};

	enum Result
	{
		PSERROR = -1,
		SAVE = 0x1,
		EXIT = 0x2,
	};

	static int show(HWND hwndParent, Configuration* pCfg,
		PageNumber pageNumber = PageNumber::TargetPage);

private:
	OptionsWindow(Configuration* pCfg);
	static int convertResult(OptionsWindow ow, INT_PTR result);
	
	OptionsPageTarget m_opTarget;
	OptionsPageMore m_opMore;
	OptionsPageUninstall m_opUninstall;

	static const int nPages = 3;
	
};