#include "stdafx.h"
#include "NotifyIcon.h"


NotifyIcon::NotifyIcon()
	: m_isCreated(false),
	  m_isIconVisible(false),
	  m_currentNotification(0),
	  m_currentIconId(0),
	  m_defaultData()
{
	m_defaultData.cbSize = sizeof(m_defaultData);
	m_defaultData.uID = m_iconId;
}



NotifyIcon::NotifyIcon(HWND hwnd) : NotifyIcon()
{
	setWindow(hwnd);
}



NotifyIcon::~NotifyIcon()
{
	hide();
}



void NotifyIcon::setWindow(HWND hwnd)
{
	if (IsWindow(hwnd))
	{
		m_defaultData.hWnd = hwnd;
		m_isCreated = true;
	}
}



void NotifyIcon::show(int icon)
{
	if (m_isCreated)
	{
		m_currentIconId = icon;
		NOTIFYICONDATA nid = m_defaultData;
		nid.uFlags = NIF_ICON;
		nid.hIcon = LoadAppIcon(icon);

		if (m_isIconVisible)
		{
			Shell_NotifyIcon(NIM_MODIFY, &nid);
		}
		else {
			nid.uFlags |= NIF_MESSAGE | NIF_SHOWTIP;
			nid.uCallbackMessage = message;
			nid.uVersion = NOTIFYICON_VERSION_4;

			if (Shell_NotifyIcon(NIM_ADD, &nid))
				m_isIconVisible = true;

			assert(Shell_NotifyIcon(NIM_SETVERSION, &nid));
		}
	}
}



void NotifyIcon::show()
{
	show(m_currentIconId);
}



void NotifyIcon::hide()
{
	if (m_isCreated)
	{
		NOTIFYICONDATA nid = m_defaultData;
		Shell_NotifyIcon(NIM_DELETE, &nid);
		m_isIconVisible = false;
	}
}



void NotifyIcon::getRect(RECT* pIconRect) const
{
	NOTIFYICONIDENTIFIER niid = {
		sizeof(niid),
		m_defaultData.hWnd,
		m_iconId,
		0
	};
	if (Shell_NotifyIconGetRect(&niid, pIconRect) != S_OK)
	{
		*pIconRect = { 0 };
	}
}



void NotifyIcon::setTip(LPCTSTR appName, LPCTSTR status)
{
	if (m_isIconVisible)
	{
		NOTIFYICONDATA nid = m_defaultData;
		nid.uFlags = NIF_TIP;
		if (status != nullptr) {
			swprintf_s(nid.szTip, 64, L"%s (%s)", appName, status);
		}
		else {
			wcscpy_s(nid.szTip, 64, appName);
		}
		Shell_NotifyIcon(NIM_MODIFY, &nid);
	}
}



void NotifyIcon::notify(LPCTSTR caption, LPCTSTR message, int iconId, int notifyId)
{
	if (m_isIconVisible)
	{
		NOTIFYICONDATA nid = m_defaultData;
		nid.uFlags = NIF_INFO;
		nid.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
		
		StringCchCopyN(nid.szInfoTitle, captionSize, caption, captionSize - 1);
		StringCchCopyN(nid.szInfo, messageSize, message, messageSize - 1);
		nid.hBalloonIcon = LoadAppIcon(iconId);

		if (Shell_NotifyIcon(NIM_MODIFY, &nid))
			m_currentNotification = notifyId;
	}
}



void NotifyIcon::notify(int captionId, int messageId, int iconId, int notifyId)
{
	if (m_isIconVisible)
	{
		if (notifyId == 0)
			notifyId = captionId;

		NOTIFYICONDATA nid = m_defaultData;
		nid.uFlags = NIF_INFO;
		nid.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;

		LoadString(GetModuleHandle(NULL), captionId, nid.szInfoTitle, captionSize);
		LoadString(GetModuleHandle(NULL), messageId, nid.szInfo, messageSize);
		nid.hBalloonIcon = LoadAppIcon(iconId);

		if (Shell_NotifyIcon(NIM_MODIFY, &nid))
			m_currentNotification = notifyId;
	}
}



void NotifyIcon::clearNotification()
{
	if (m_isIconVisible)
	{
		NOTIFYICONDATA nid = m_defaultData;
		nid.uFlags = NIF_INFO;
		Shell_NotifyIcon(NIM_MODIFY, &nid);
		m_currentNotification = 0;
	}
}



bool NotifyIcon::isNotificationSet() const
{
	return m_currentNotification != 0;
}

int NotifyIcon::getCurrentNotification() const
{
	return m_currentNotification;
}
