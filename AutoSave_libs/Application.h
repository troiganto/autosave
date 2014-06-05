// Application.h : Defines the main window and its window procedure.
// Must absorb all exceptions.
//

#pragma once

#include "stdafx.h"
#include "Configuration.h"
#include "OptionsWindow.h"
#include "ShortcutsDisconnector.h"
#include "NotifyIcon.h"
#include "PeriodicSender.h"
#include "BaseWindow.h"
#include "..\AutoSave\\Resource.h"

using std::wstring;

class Application : public BaseWindow
{
public:
	Application(LPCTSTR pCmdLine);
	~Application();

	// Overwrite inherited member function.
	void registerWindowClass();
	static const LPTSTR windowClassName;

	static void mainLoop();

protected:
	// Implement purely virtual inherited member function.
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Event handlers
	void OnNotifyIcon(WORD msg, WORD iconId, int x, int y);
	static DWORD CALLBACK onNotifyLClickThread(LPVOID lParam);
	struct NOTIFYTHREADARGUMENTS {
		Application* pThis;
		bool* pIsWaitingForSecondClick;
		WORD iconId;
		int x;
		int y;
	};
	enum {WM_LBUTTONCLICKEDONCE = WM_USER + 0x000A};

	void OnCreate();
	void OnTimer();
	void OnDestroy();

	void onNotifyIconLClick(WORD iconId, int x, int y);
	void onNotifyIconLDblClk(WORD iconId, int x, int y);
	void onNotifyIconRClick(WORD iconId, int x, int y);
	void onNotifyBubbleClicked(int notifId);
	
	void onShortcutMenuClicked(int menuId);

	void onSenderDelayAtZero();
	void onSenderAtFive(UINT_PTR secondsLeft);
	void onSenderAtLessThanFive(UINT_PTR secondsLeft);
	void onSenderAtZero();

private:
	void initConfiguration();
	static wstring getStartingShortcutFileName();

	// Lower-level stuff.
	int trackShortcutMenu(int x, int y);
	void switchToOptionsWindow(
		OptionsWindow::PageNumber pageNumber =
			OptionsWindow::PageNumber::TargetPage);
	void switchToBeingEnabled();
	void switchToBeingDisabled();
	void showOptionsWindow(OptionsWindow::PageNumber pageNumber,
		bool* pShallSave, bool* pShallExit);
	void shutdown();

	wstring m_commandLine;
	Configuration m_cfg;
	NotifyIcon m_icon;
	PeriodicSender m_sender;
	HMENU m_hContextMenu;

};

