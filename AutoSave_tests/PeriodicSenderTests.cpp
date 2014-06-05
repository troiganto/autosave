#include "stdafx.h"
#include "CppUnitTest.h"
#include "PeriodicSender.h"
#include "AppConnection.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace AutoSave_tests
{
	const wstring notepadPath = L"C:\\Windows\\System32\\notepad.exe";

	void closeNotepad(const AppConnection& ac)
	{
		Sleep(50);
		vector<HWND> windows = ac.getConnectedWindows();
		Assert::AreEqual(1, (int)windows.size(), L"wrong number of windows");
		PostMessage(windows[0], WM_CLOSE, 0, 0);
		int timeout = 4;
		while (timeout > 0 && IsWindow(windows[0]))
		{
			Sleep(10);
			--timeout;
		}
		if (timeout == 0)
			Assert::Fail(L"notepad took to long to shut down.");
	}

	TEST_CLASS(SenderTests)
	{
	public:
		
		TEST_METHOD(TestSendingInputs)
		{
			AppConnection ac;
			ac.connect(notepadPath);
			if (ac.waitForInputIdle(1000) == WAIT_TIMEOUT)
				Assert::Fail(L"notepad took too long to start up");
			HWND notepad = ac.getConnectedWindows()[0];
			{
				ShowWindow(notepad, SW_SHOW);
				int timeout = 4;
				while (timeout > 0 && !IsWindowVisible(notepad))
				{
					Sleep(10);
					--timeout;
				}
				if (timeout == 0)
					Assert::Fail(L"notepad took too long to become visible");
				SetForegroundWindow(notepad);
			}

			Sleep(1000);
			Assert::AreEqual(2,
				(int)PeriodicSender::sendKeys(MAKEWORD(0x54, HOTKEYF_SHIFT)),
				L"T didn't work");
			Assert::AreEqual(1,
				(int)PeriodicSender::sendKeys(MAKEWORD(0x45, 0)),
				L"e didn't work");
			Assert::AreEqual(2,
				(int)PeriodicSender::sendKeys(MAKEWORD(0x53, HOTKEYF_CONTROL)),
				L"ctrl+s didn't work");

			Sleep(1000);
			Assert::AreEqual(1,
				(int)PeriodicSender::sendKeys(MAKEWORD(VK_ESCAPE, 0)),
				L"esc didn't work");

			Sleep(1000);
			if (GetForegroundWindow() != notepad)
				Assert::Fail(L"wrong foreground window!");
			Assert::AreEqual(2,
				(int)PeriodicSender::sendKeys(MAKEWORD(VK_F4, HOTKEYF_ALT)),
				L"alt+f4 didn't work");
			Sleep(1000);
			Assert::AreEqual(2,
				(int)PeriodicSender::sendKeys(MAKEWORD(0x4E, HOTKEYF_ALT)),
				L"alt+n didn't work");
			Sleep(1000);
			Assert::IsFalse(ac.isConnectionAlive(),
				L"Notepad is still open!");
		}

	};
}