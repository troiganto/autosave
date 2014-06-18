// Sender.h : Interface for a timer and actually sending
// keyboard input to other applications.
// Never throws exceptions.

#pragma once

#include "stdafx.h"

using std::vector;

class PeriodicSender
{
public:
	PeriodicSender(UINT interval) : m_hwnd(0), m_interval(interval) {}
	~PeriodicSender() { stop(); }

	void setWindow(HWND hwnd);

	inline void setInterval(UINT interval) { m_interval = interval; }

	void start();
	void stop();
	void step();

	void pause();
	void resume();

	void resetCountdown();
	void resetDelay();

	static UINT sendKeys(WORD hotkey);
	static bool noKeyPressed();

	enum SenderMessage {
		SM_START = WM_USER + 0x0100,
		SM_DELAYATZERO,
		SM_FIVESECONDSLEFT,
		SM_LESSTHANFIVELEFT,
		SM_ATZERO
	};

private:
	static void insertKey(vector<INPUT>* pList, WORD key);

	HWND m_hwnd;
	UINT m_interval;

	bool m_isStarted;
	bool m_isPaused;
	UINT m_mainSecondsLeft;
	UINT m_delaySecondsLeft;

	static const UINT delayTime = 2;
	static const UINT_PTR timerId = 622;
};

