// BaseWindow.h : Base class for application windows.

#pragma once

class BaseWindow
{
public:
	BaseWindow();
	~BaseWindow();

	void registerWindowClass();
	static const LPTSTR windowClassName;
	bool create();

	inline bool successfullyCreated() const { return m_hwnd != 0; }
	inline void showWindow(int nCmdShow) const { ShowWindow(m_hwnd, nCmdShow); }

protected:
	static LRESULT CALLBACK windowProc(
		HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;


	ATOM m_classAtom;
	HWND m_hwnd;
};

