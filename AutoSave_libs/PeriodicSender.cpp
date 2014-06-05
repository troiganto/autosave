#include "stdafx.h"
#include "PeriodicSender.h"


void PeriodicSender::setWindow(HWND hwnd)
{
	if (!m_isStarted)
		m_hwnd = hwnd;
}


void PeriodicSender::start()
{
	if (m_hwnd != 0)
	{
		SetTimer(m_hwnd, timerId, 1000, NULL);
		PostMessage(m_hwnd, SM_START, 0, 0);
	}

	m_isStarted = true;
	m_isPaused = false;
	m_mainSecondsLeft = m_interval;
}



void PeriodicSender::stop()
{
	if (m_isStarted)
	{
		if (m_hwnd!= 0 && !m_isPaused)
			KillTimer(m_hwnd, timerId);

		m_isStarted = false;
		m_isPaused = false;
		m_mainSecondsLeft = 0;
	}
}



void PeriodicSender::pause()
{
	if (m_isStarted && !m_isPaused)
	{
		m_isPaused = true;
		if (m_hwnd != 0)
			KillTimer(m_hwnd, timerId);
	}
}



void PeriodicSender::resume()
{
	if (m_isPaused)
	{
		m_isPaused = false;
		if (m_hwnd != 0)
			SetTimer(m_hwnd, timerId, 1000, NULL);
	}
}



void PeriodicSender::step()
{
	if (m_isPaused || !m_isStarted)
		return;

	if (m_delaySecondsLeft > 0)
	{
		// If a delay has been specified, wait before
		// doing the actual countdown.
		--m_delaySecondsLeft;
		if (m_delaySecondsLeft == 0 && m_hwnd != 0)
			PostMessage(m_hwnd, SM_DELAYATZERO, 0, 0);
	}
	else
	{
		// Actual countdown.
		if (m_mainSecondsLeft > 0)
			--m_mainSecondsLeft;

		if (m_hwnd != 0)
		{
			if (m_mainSecondsLeft <= 0)
				PostMessage(m_hwnd, SM_ATZERO, 0, 0);
			else if (m_mainSecondsLeft < 5)
				PostMessage(m_hwnd, SM_LESSTHANFIVELEFT,
							m_mainSecondsLeft, 0);
			else if (m_mainSecondsLeft == 5)
				PostMessage(m_hwnd, SM_FIVESECONDSLEFT, 5, 0);
		}
	}
}



void PeriodicSender::resetCountdown()
{
	m_mainSecondsLeft = m_interval;
}



void PeriodicSender::resetDelay()
{
	m_delaySecondsLeft = delayTime;
}



UINT PeriodicSender::sendKeys(WORD hotkey)
{
	if (LOBYTE(hotkey) == 0)
		return 0;

	vector<INPUT> inputs;
	// Get space for up&down events for one key and up to three modifiers.
	inputs.reserve(8);
	if (HIBYTE(hotkey) & HOTKEYF_CONTROL)
		insertKey(&inputs, VK_CONTROL);
	if (HIBYTE(hotkey) & HOTKEYF_SHIFT)
		insertKey(&inputs, VK_SHIFT);
	if (HIBYTE(hotkey) & HOTKEYF_ALT)
		insertKey(&inputs, VK_MENU);
	insertKey(&inputs, LOBYTE(hotkey));
	
	int inputsSent = SendInput((UINT) inputs.size(), inputs.data(), sizeof(INPUT));
	return inputsSent / 2;
}

void PeriodicSender::insertKey(vector<INPUT>* pList, WORD key)
{
	const size_t middle = pList->size() / 2;

	INPUT input = { 0 };
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = key;
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	pList->insert(pList->begin() + middle, input);
	input.ki.dwFlags = 0;
	pList->insert(pList->begin() + middle, input);
}