#include "stdafx.h"
#include "BaseWindow.h"


const LPTSTR BaseWindow::windowClassName = APP_NAME L" base window class";

BaseWindow::BaseWindow()
{
}


BaseWindow::~BaseWindow()
{
}



void BaseWindow::registerWindowClass()
{
	if (m_classAtom != 0)
		return;

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof wc;
	wc.lpfnWndProc = BaseWindow::windowProc;
	wc.hInstance = GetModuleHandle(NULL);
	//wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(iconId));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = windowClassName;

	m_classAtom = RegisterClassEx(&wc);
}



bool BaseWindow::create()
{
	// m_hwnd will be set by the window procedure
	CreateWindowEx(
		0,
		MAKEINTATOM(m_classAtom),
		APP_NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0,
		0,
		GetModuleHandle(NULL),
		this
		);
	return successfullyCreated();
}



LRESULT CALLBACK BaseWindow::windowProc(
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BaseWindow* pThis = NULL;

	if (uMsg == WM_NCCREATE)
	{
		auto pCreate = (CREATESTRUCT*)lParam;
		pThis = (BaseWindow*)pCreate->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
		// Set the m_hwnd member as early as possible.
		pThis->m_hwnd = hwnd;
	}
	else {
		pThis = (BaseWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (pThis)
	{
		return pThis->HandleMessage(uMsg, wParam, lParam);
	}
	else {
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}
