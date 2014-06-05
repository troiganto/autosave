#include "stdafx.h"
#include "UninstallerShortcutsListTooltip.h"



UninstallerShortcutsListTooltip::UninstallerShortcutsListTooltip()
{
	INITCOMMONCONTROLSEX icce = {
		sizeof(icce), ICC_BAR_CLASSES };
	InitCommonControlsEx(&icce);

	m_tooltip = CreateWindowEx(
		0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_BALLOON,
		0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);

	m_tooltipInfo = {
		sizeof(m_tooltipInfo), TTF_ABSOLUTE | TTF_TRACK,
		NULL, 0, { 0, 0, 0, 0 },
		GetModuleHandle(NULL), MAKEINTRESOURCE(IDS_ERROR_NODROPPEDSHORTCUTS),
		NULL, NULL };
}



UninstallerShortcutsListTooltip::~UninstallerShortcutsListTooltip()
{
	DestroyWindow(m_tooltip);
}



void UninstallerShortcutsListTooltip::init(HWND hwndDialog, HWND hwndTool)
{
	m_tooltipInfo.hwnd = hwndDialog;
	m_tooltipInfo.uId = (UINT_PTR)hwndTool;
	GetClientRect(hwndTool, &m_tooltipInfo.rect);
	SendMessage(m_tooltip, TTM_ADDTOOL, 0, (LPARAM)&m_tooltipInfo);
	SendMessage(m_tooltip, TTM_ACTIVATE, TRUE, 0);
}



void UninstallerShortcutsListTooltip::show(const POINTL& pt)
{
	KillTimer((HWND)m_tooltipInfo.uId, timerId);
	SendMessage(m_tooltip, TTM_TRACKPOSITION, 0, MAKELONG(pt.x, pt.y));
	SendMessage(m_tooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_tooltipInfo);
	SetTimer((HWND)m_tooltipInfo.uId, timerId, showDelay, NULL);

}



void UninstallerShortcutsListTooltip::hide()
{
	SendMessage(m_tooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_tooltipInfo);
	KillTimer((HWND)m_tooltipInfo.uId, timerId);
}
