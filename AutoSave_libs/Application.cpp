
#include "stdafx.h"
#include "Application.h"

const LPTSTR Application::windowClassName = APP_NAME L" main window class";

Application::Application(LPCTSTR pCmdLine)
	: m_commandLine(pCmdLine),
	  m_sender(m_cfg.settings.getInterval())
{
	OleInitialize(NULL);
}


Application::~Application()
{
	OleUninitialize();
}



void Application::registerWindowClass()
{
	if (m_classAtom != 0)
		return;

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof wc;
	wc.lpfnWndProc = Application::windowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_A));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = windowClassName;

	m_classAtom = RegisterClassEx(&wc);
}



void Application::mainLoop()
{
	MSG msg = {};
	BOOL getMessageResult;
	while ((getMessageResult = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (getMessageResult == -1)
		{
			log(L"Fatal error! Variable getMessageResult is -1!");
			log(L"How could this happend to meeeeeeee~~~");
			break;
		}
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}



LRESULT Application::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	try {
		switch (uMsg)
		{
		case WM_CREATE:
			OnCreate();
			return 0;

		case WM_DESTROY:
			OnDestroy();
			PostQuitMessage(0);
			return 0;

		case WM_TIMER:
			OnTimer();
			return 0;

		case NotifyIcon::message: {
			POINT p;
			m_icon.estimateCursorPos(&p);
			OnNotifyIcon(LOWORD(lParam), HIWORD(lParam),
				p.x, p.y);
			return 0;
		}

		case PeriodicSender::SM_ATZERO:
			onSenderAtZero();
			return 0;
		case PeriodicSender::SM_LESSTHANFIVELEFT:
			onSenderAtLessThanFive(wParam);
			return 0;
		case PeriodicSender::SM_FIVESECONDSLEFT:
			onSenderAtFive(wParam);
			return 0;
		case PeriodicSender::SM_DELAYATZERO:
			onSenderDelayAtZero();
			return 0;
		default:
			return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
		}
	}
	catch (AutoSaveException& exc) {
		// Emergency exception handler.
		exc.showMessageBox(m_hwnd, L"An unexpected error occured.");
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}
	catch (std::exception& exc) {
		AutoSaveException::showMessageBox(
			m_hwnd, L"An unexpected error occured.", exc);
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}
}



void Application::OnNotifyIcon(WORD msg, WORD iconId, int x, int y)
{
	// This bool is both a mutex and a signaling tool to distinguish
	// between a single and a double left click.
	static bool isWaitingForSecondClick;

	if (msg == NIN_BALLOONTIMEOUT)
	{
		m_icon.clearNotification();
	}
	else if (msg == NIN_BALLOONUSERCLICK)
	{
		onNotifyBubbleClicked(m_icon.getCurrentNotification());
		m_icon.clearNotification();
	}
	else if (msg == WM_LBUTTONDBLCLK)
	{
		isWaitingForSecondClick = false;
		onNotifyIconLDblClk(iconId, x, y);
	}
	else if (msg == WM_LBUTTONCLICKEDONCE)
	{
		onNotifyIconLClick(iconId, x, y);
	}
	else if (msg == WM_LBUTTONDOWN && !isWaitingForSecondClick)
	{
		isWaitingForSecondClick = true;
		auto pArgs = new NOTIFYTHREADARGUMENTS;
		*pArgs = { this, &isWaitingForSecondClick, iconId, x, y };
		CloseHandle(CreateThread(NULL, 12, onNotifyLClickThread, pArgs, 0, NULL));
	}
	else if (msg == WM_RBUTTONUP)
	{
		isWaitingForSecondClick = false;
		onNotifyIconRClick(iconId, x, y);
	}
}

DWORD CALLBACK Application::onNotifyLClickThread(LPVOID lParam)
{
	// We copy the struct pointed to by lParam because it might
	// cease to exist once the other function exits.
	auto pArgs = (NOTIFYTHREADARGUMENTS*) lParam;
	Sleep(GetDoubleClickTime());
	if (*pArgs->pIsWaitingForSecondClick)
	{
		PostMessage(pArgs->pThis->m_hwnd, NotifyIcon::message,
			MAKELONG(pArgs->x, pArgs->y),
			MAKELONG(WM_LBUTTONCLICKEDONCE, pArgs->iconId));
	}
	*pArgs->pIsWaitingForSecondClick = false;
	delete pArgs;
	return 0;
}



void Application::OnCreate()
{
	initConfiguration();
	wstring startingShortcut = getStartingShortcutFileName();
	if (!startingShortcut.empty())
		ShortcutsDisconnector::registerConnectedShortcuts(startingShortcut);


	m_hContextMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_MENU));
	m_sender.setWindow(m_hwnd);
	m_icon.setWindow(m_hwnd);

	if (m_cfg.connection.isConnected())
	{
		switchToBeingEnabled();
	}
	else if (m_cfg.isFirstSession)
	{
		switchToOptionsWindow();
	}
	else {
		switchToBeingEnabled();
	}
}

void Application::OnTimer()
{
	// Quit when the connected app is closed.
	if (m_cfg.connection.isConnected() && !m_cfg.connection.isConnectionAlive())
	{
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
	}
	else {
		m_sender.step();
	}
}

void Application::OnDestroy()
{
	DestroyMenu(m_hContextMenu);

	m_icon.hide();
	m_sender.stop();
}



void Application::onNotifyIconLClick(WORD iconId, int x, int y)
{
	onShortcutMenuClicked(trackShortcutMenu(x, y));
}

void Application::onNotifyIconLDblClk(WORD iconId, int x, int y)
{
	if (!m_cfg.connection.isConnected())
		switchToOptionsWindow();
}

void Application::onNotifyIconRClick(WORD iconId, int x, int y)
{
	onShortcutMenuClicked(trackShortcutMenu(x, y));
}

void Application::onNotifyBubbleClicked(int notifyId)
{
	if (notifyId == IDS_ALERT_CAPTION)
	{
		switchToOptionsWindow(OptionsWindow::PageNumber::MoreOptionsPage);
	}
	else if (notifyId == IDS_DISABLED_CAPTION)
	{
		switchToOptionsWindow(OptionsWindow::PageNumber::TargetPage);
	}
}



void Application::onShortcutMenuClicked(int menuId)
{
	switch (menuId)
	{
	case IDM_OPTIONS:
		switchToOptionsWindow();
		break;
	case IDM_ENABLE:
		if (!m_cfg.isEnabled)
			switchToBeingEnabled();
		break;
	case IDM_DISABLE:
		if (m_cfg.isEnabled)
			switchToBeingDisabled();
		break;
	case IDM_CLOSE:
		shutdown();
		break;
	default:
		break;
	}
}



void Application::onSenderDelayAtZero()
{
	m_icon.show(IDI_A);
}

void Application::onSenderAtFive(UINT_PTR secondsLeft)
{
	if (!m_cfg.matchingWindowExists())
	{
		m_sender.resetCountdown();
		m_icon.show(IDI_A);
	}
	else
	{
		if (m_cfg.settings.verbosityExceeds(MiscSettings::ALERT_FIVE_SECONDS))
		{
			m_icon.notify(IDS_ALERT_CAPTION, IDS_ALERT_TEXT, IDI_A);
		}
		if (m_cfg.settings.verbosityExceeds(MiscSettings::SHOW_ICONS))
		{
			m_icon.show(IDI_COUNT5);
		}
	}
}

void Application::onSenderAtLessThanFive(UINT_PTR secondsLeft)
{
	if (m_cfg.settings.verbosityExceeds(MiscSettings::SHOW_ICONS))
	{
		switch (secondsLeft)
		{
		case 4: m_icon.show(IDI_COUNT4); break;
		case 3: m_icon.show(IDI_COUNT3); break;
		case 2: m_icon.show(IDI_COUNT2); break;
		case 1: m_icon.show(IDI_COUNT1); break;
		default: break;
		}
	}
}

void Application::onSenderAtZero()
{
	if (m_cfg.windowMatch(GetForegroundWindow()))
	{
		PeriodicSender::sendKeys(m_cfg.settings.getHotkey());
		m_sender.resetCountdown();
		if (m_cfg.settings.verbosityExceeds(MiscSettings::SHOW_ICONS))
		{
			m_sender.resetDelay();
			// Clear potential five-seconds alert.
			m_icon.clearNotification();
			m_icon.show(IDI_OK);
		}
	}
	else if (!m_cfg.matchingWindowExists())
	{
		m_sender.resetCountdown();
		m_icon.clearNotification();
		m_icon.show(IDI_A);
	}
	else if (m_cfg.settings.verbosityExceeds(MiscSettings::SHOW_ICONS))
	{
		m_icon.show(IDI_COUNT0);
	}
}



void Application::initConfiguration()
{
	try {
		m_cfg.loadFromRegistry(DEFAULT_REGISTRY_KEY);
	}
	catch (AutoSaveException& exc) {
		exc.showMessageBox(0, L"Couldn't load app settings.");
		m_cfg.isFirstSession = true;
	}
	catch (std::exception& exc) {
		AutoSaveException::showMessageBox(0, L"Couldn't load app settings.", exc);
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
	}

	try {
		m_cfg.loadFromCommandLine(m_commandLine);
	}
	catch (AutoSaveException& exc) {
		exc.showMessageBox(0,
			L"Couldn't read the command-line arguments passed to "
			SHORT_APP_NAME L".");
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
	}
	catch (std::exception& exc) {
		AutoSaveException::showMessageBox(0,
			L"Couldn't read the command-line arguments passed to "
			SHORT_APP_NAME L".", exc);
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
	}
}



wstring Application::getStartingShortcutFileName()
{
	STARTUPINFO si;
	GetStartupInfo(&si);
	return (si.dwFlags & STARTF_TITLEISLINKNAME) ? si.lpTitle : L"";
}



int Application::trackShortcutMenu(int x, int y)
{
	m_sender.pause();
	HMENU contextMenu = GetSubMenu(m_hContextMenu, 0);
	if (m_cfg.connection.isConnected())
	{
		RemoveMenu(contextMenu, IDM_OPTIONS, MF_BYCOMMAND);
	}
	else {
		RemoveMenu(contextMenu, IDM_NOOPTIONS, MF_BYCOMMAND);
	}
	CheckMenuItem(contextMenu, IDM_ENABLE,
		MF_BYCOMMAND | (m_cfg.isEnabled ? MFS_CHECKED : MF_UNCHECKED));
	CheckMenuItem(contextMenu, IDM_DISABLE,
		MF_BYCOMMAND | (m_cfg.isEnabled ? MF_UNCHECKED : MFS_CHECKED));

	UINT flags = TPM_NONOTIFY | TPM_RETURNCMD;
	// Maybe consider adding the following line at some point.
	// The MSDN suggests it, but the results seem less pleasant.
	// flags |= (GetSystemMetrics(SM_MENUDROPALIGNMENT) ? TPM_RIGHTALIGN : TPM_LEFTALIGN);

	// SetForegroundWindow and WM_NULL are suggested by the MSDN.
	SetForegroundWindow(m_hwnd);
	int result = TrackPopupMenu(
		contextMenu, flags, x, y, 0, m_hwnd, NULL);
	PostMessage(m_hwnd, WM_NULL, 0, 0);

	m_sender.resume();
	return result;
}



void Application::switchToOptionsWindow(OptionsWindow::PageNumber pageNumber)
{
	// The showOptionsWindow function will manipulate m_icon and m_sender.
	// This will be taken care of below.
	bool shallSave, shallExit, shallEnable;
	showOptionsWindow(pageNumber, &shallSave, &shallExit);
	shallEnable = m_cfg.isEnabled || shallSave;

	if (shallSave) {
		try {
			m_cfg.saveToRegistry(DEFAULT_REGISTRY_KEY);
		}
		catch (AutoSaveException& exc) {
			exc.showMessageBox(0, L"Couldn't save app settings.");
		}
	}

	// The showOptionsWindow function has manipulated m_icon and m_sender.
	// Here, we take care of that.
	if (shallExit) {
		shutdown();
	}
	else if (shallEnable) {
		switchToBeingEnabled();
	}
	else {
		switchToBeingDisabled();
	}
}



void Application::switchToBeingEnabled()
{
	if (m_cfg.canRun())
	{
		m_icon.show(IDI_A);
		m_icon.setTip(APP_NAME, L"Running");
		m_sender.setInterval(m_cfg.settings.getInterval());
		m_sender.start();
		m_cfg.isEnabled = true;
		if (m_cfg.settings.verbosityExceeds(MiscSettings::ALERT_START))
		{
			m_icon.notify(IDS_STARTED_CAPTION, IDS_STARTED_TEXT, IDI_A);
		}
	}
	else {
		switchToBeingDisabled();
		m_icon.notify(IDS_DISABLED_CAPTION, IDS_DISABLED_TEXT, IDI_DISABLED);
	}
}

void Application::switchToBeingDisabled()
{
	m_icon.show(IDI_DISABLED);
	m_icon.setTip(APP_NAME, L"Disabled");
	m_sender.stop();
	m_cfg.isEnabled = false;
}



void Application::showOptionsWindow(OptionsWindow::PageNumber pageNumber,
	bool* pShallSave, bool* pShallExit)
{
	m_icon.hide();
	if (!m_cfg.connection.isConnected())
	{
		m_sender.stop();
		int optionsResult = OptionsWindow::show(0, &m_cfg, pageNumber);
		if (optionsResult == OptionsWindow::Result::PSERROR)
		{
			AutoSaveException exc;
			exc.showMessageBox(m_hwnd, L"Couldn't show options window.");
			*pShallSave = false;
			*pShallExit = false;
		}
		else {
			*pShallSave = (optionsResult & OptionsWindow::Result::SAVE) != 0;
			*pShallExit = (optionsResult & OptionsWindow::Result::EXIT) != 0;
		}
		// Don't re-start sender. The SwitchToBeing*abled functions will do that.
	}
	else {
		m_sender.pause();
		MessageBox(0, L"Settings set by a connected shortcut cannot be changed.",
			APP_NAME, MB_ICONERROR | MB_TASKMODAL);
		m_sender.resume();
		*pShallSave = false;
		*pShallExit = false;
	}
	// Don't re-show icon. The SwitchToBeing*abled functions will do that.
}



void Application::shutdown()
{
	bool doClose = true;
	if (m_cfg.connection.isConnectionAlive())
	{
		m_icon.hide();
		m_sender.pause();
		doClose = IDYES == MessageBox(0, L"The connected application is "
			L"still running! Do you really want to shut down " SHORT_APP_NAME L"?",
			APP_NAME, MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION | MB_TASKMODAL);
		m_sender.resume();
		// Don't re-show icon when we're gonna shut down anyway.
	}
	if (doClose)
	{
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
	}
	else {
		m_icon.show();
	}
}
