#include "stdafx.h"
#include "CppUnitTest.h"
#include "AppConnection.h"
#include "TemporaryShortcutFile.h"
#include "RegistryAccess.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AutoSave_tests
{
	TEST_CLASS(AppConnectionTests)
	{
	public:
		
		AppConnection ac;
		TemporaryShortcutFile tsf;
		const wstring notepad = LR"(C:\Windows\System32\notepad.exe)";
		const wstring dir = LR"(C:\dev\autosave\autosave test files\ac\)";
		const wstring file = dir + L"file.txt";
		const wstring weirdExt = dir + L"file.autosavetesttype";
		const wstring link = dir + L"link.lnk";
		const wstring connected = dir + L"link + AutoSave.lnk";

		void closeConnectedProcess(const AppConnection& ac)
		{
			vector<HWND> windows = ac.getConnectedWindows();
			Assert::AreEqual<size_t>(1, windows.size(),
				L"wrong number of windows");
			PostMessage(windows[0], WM_CLOSE, 0, 0);

			int timeout = 4;
			while (IsWindow(windows[0]) && timeout > 0)
			{
				Sleep(10);
				--timeout;
			}
			if (timeout == 0)
				Assert::Fail(L"Didn't close window");
		}

		TEST_CLASS_INITIALIZE(InitClass)
		{
			vector<wstring> names = {
				L"classes\\.autosavetesttype",
				L"classes\\.autosavetesttype\\shell",
				L"classes\\.autosavetesttype\\shell\\open",
				L"classes\\.autosavetesttype\\shell\\open\\command"
			};
			RegistryAccess ra;
			for (const wstring& n : names)
			{
				ra.access(n);
			}
			TCHAR buffer[MAX_PATH];
			StringCchPrintf(buffer, MAX_PATH, LR"("%s" "%%1")",
				OleUtils::getSelfPath().data());
			ra.writeString(L"", buffer);
		}

		TEST_CLASS_CLEANUP(ExitClass)
		{
			RegistryAccess ra;
			ra.access(L"classes\\.autosavetesttype");
			ra.purge();
		}

		TEST_METHOD(TestACCopyConstructors)
		{
			try {
				DWORD handleCount = 0;

				AppConnection acone;
				Assert::AreEqual<DWORD>(0, acone.getProcessId());

				AppConnection actwo(acone);
				Assert::AreEqual<DWORD>(0, actwo.getProcessId());

				acone.connect(notepad);
				if (acone.waitForInputIdle(1000) == WAIT_TIMEOUT)
					Assert::Fail(L"notepad took too long to start up");
				actwo = acone;
				Assert::AreEqual<bool>(
					acone.isConnectionAlive(), actwo.isConnectionAlive(),
					L"check before closing");

				closeConnectedProcess(acone);
				Assert::AreEqual<bool>(
					acone.isConnectionAlive(), actwo.isConnectionAlive(),
					L"check after closing");

				actwo.disconnect();
				Assert::AreEqual<DWORD>(0, actwo.getProcessId());
			}
			catch (AutoSaveException& exc) {
				Logger::WriteMessage(exc.wcwhat());
				throw;
			}
		}

		TEST_METHOD(TestACFailure)
		{
			const wstring nonexisting = LR"(C:\non.existing)";
			Assert::ExpectException<std::runtime_error>([&nonexisting]() {
				AppConnection iac;
				iac.connect(nonexisting);
			});
			try {
				vector<wstring> badArg = { notepad, nonexisting };
				ac.connect(badArg);
			}
			catch (AutoSaveException& exc) {
				Logger::WriteMessage(exc.wcwhat());
				throw;
			}
			if (ac.waitForInputIdle(1000) == WAIT_TIMEOUT)
				Assert::Fail(L"notepad took too long to start up");
			closeConnectedProcess(ac);
		}

		TEST_METHOD(TestACConnectToNormalThings)
		{
			AppConnection ac;
			vector<wstring> files = { notepad, file };
			wstring errorMsg;

			for (const wstring& file : files)
			{
				try {
					ac.connect(file);
				}
				catch (AutoSaveException& exc)
				{
					Logger::WriteMessage(exc.wcwhat());
					throw;
				}
				if (ac.waitForInputIdle(1000) == WAIT_TIMEOUT)
					Assert::Fail(L"notepad took too long to start up");
				Assert::IsTrue(ac.isConnectionAlive(), file.data());
				closeConnectedProcess(ac);
				Assert::IsFalse(ac.isConnectionAlive(), file.data());
				ac.disconnect();
				Assert::IsFalse(ac.isConnected(), file.data());
			}
		}

		TEST_METHOD(TestACConnectWithCLArguments)
		{
			vector<wstring> args = { notepad, file };
			ac.connect(args);
			if (ac.waitForInputIdle(1000) == WAIT_TIMEOUT)
				Assert::Fail(L"notepad took too long to start up");
			Assert::IsTrue(ac.isConnectionAlive());
			closeConnectedProcess(ac);
			Assert::IsFalse(ac.isConnectionAlive());
			ac.disconnect();
			Assert::IsFalse(ac.isConnected());
		}

		TEST_METHOD(TestACThrowIfStartingSelf)
		{
			// Set up connected shortcut
			tsf.setSaveDirectory(dir);
			tsf.updateFile(link);
			Assert::IsTrue(PathFileExists(connected.data()) != FALSE,
				tsf.getFile().data());
			
			// Actual test.
			vector<pair<wstring, bool>> testFiles = {
				{ OleUtils::getSelfPath(), true },
				{ file, false },
				{ weirdExt, true },
				{ notepad, false },
			};
			for (const pair<wstring, bool> p : testFiles)
			{
				try {
					AppConnection::throwIfStartingSelf(p.first);
					if (p.second)
						Assert::Fail(p.first.data());
				}
				catch (std::runtime_error) {
					if (!p.second)
						throw;
				}
			}
		}


	};
}