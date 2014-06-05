#include "stdafx.h"
#include "OptionsWindow.h"

// Beware that the useless boilerplate functions come first.
// Actual handling of messages is further below.

OptionsWindow::OptionsWindow(Configuration* pCfg)
	: m_opTarget(&pCfg->filter), m_opMore(&pCfg->settings), m_opUninstall()
{
}

OptionsWindow::~OptionsWindow()
{
}



int OptionsWindow::show(HWND hwndParent, Configuration* pCfg,
	PageNumber pageNumber)
{
	OptionsWindow ow(pCfg);
	HPROPSHEETPAGE phpages[nPages] = {
		ow.m_opTarget.create(),
		ow.m_opMore.create(),
		ow.m_opUninstall.create()
	};

	// We use LoadAppIcon for hIcon instead of just
	// specifying the resourceto make sure the large
	// icon is used. Otherwise, the task bar icon
	// would end up pixelated.
	PROPSHEETHEADER psh = {
		sizeof(psh),		// dwSize
		PSH_PROPTITLE | PSH_USEHICON | PSH_NOCONTEXTHELP,
							// dwFlags
		hwndParent,			// hwndParent
		GetModuleHandle(NULL),
							// hInstance
		LoadAppIcon(IDI_A),	// hIcon
		APP_NAME,			// pszCaption
		nPages,				// nPages
		(int)pageNumber,	// nStartPage
		(LPPROPSHEETPAGE) phpages,
							// *phpage
		NULL,				// pfnCallback
		NULL,				// pszbmWatermark
		NULL,				// hplWatermark
		NULL				// pszbmHeader
	};
	return convertResult(ow, PropertySheet(&psh));
}



int OptionsWindow::convertResult(OptionsWindow ow, INT_PTR result)
{
	if (result == -1)
	{
		return Result::PSERROR;
	}
	else if (ow.m_opUninstall.shouldAppExit)
	{
		return Result::EXIT;
	}
	else if (ow.m_opMore.shouldAppExit)
	{
		return Result::SAVE | Result::EXIT;
	}
	else if (result > 0)
	{
		return Result::SAVE;
	}
	else {
		return 0;
	}
}