// NotifyIcon.h : Handles Windows' clumsy interface
// to notification area icons.
// Never throws exceptions.

#pragma once

#include "stdafx.h"

class NotifyIcon
{
public:
	NotifyIcon();
	NotifyIcon(HWND hwnd);
	~NotifyIcon();

	void setWindow(HWND hwnd);

	void show(int icon);
	void show();
	void hide();

	void getRect(RECT* pIconRect) const;
	void estimateCursorPos(POINT* pPoint) const;

	void setTip(LPCTSTR appName, LPCTSTR status = nullptr);

	void notify(LPCTSTR caption, LPCTSTR message, int iconId, int notifyId);
	void notify(int captionId, int messageId, int iconId, int notifyId = 0);
	void clearNotification();
	bool isNotificationSet() const;
	int getCurrentNotification() const;

	static const UINT message = WM_USER + 1;

private:
	bool m_isCreated;
	bool m_isIconVisible;
	int m_currentNotification;
	int m_currentIconId;
	NOTIFYICONDATA m_defaultData;

	static const int m_iconId = 1;
	static const int captionSize = 64;
	static const int messageSize = 256;
};

