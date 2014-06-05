#include "stdafx.h"
#include "CppUnitTest.h"
#include "Configuration.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AutoSave_tests
{
	TEST_CLASS(ConfigurationTests)
	{
	public:

		const wstring regKey = L"Broken AutoSave Configuration Test";

		void closeConnectedProcess(const Configuration& cfg)
		{
			const AppConnection& ac = cfg.connection;
			Assert::IsTrue(ac.isConnectionAlive());

			vector<HWND> hwnds = ac.getConnectedWindows();
			Assert::AreEqual<size_t>(1, hwnds.size());
			HWND hwnd = hwnds[0];
			PostMessage(hwnd, WM_CLOSE, 0, 0);

			int timeout = 4;
			while (timeout > 0 && IsWindow(hwnd))
			{
				Sleep(10);
				--timeout;
			}
			if (timeout == 0)
				Assert::Fail(L"app took too long to shut down.");
		}



		TEST_METHOD(TestCfgRegistryAccess)
		{
			Configuration cfg;
			cfg.settings.setInterval(77);
			cfg.saveToRegistry(regKey.data());

			Configuration otherCfg;
			otherCfg.loadFromRegistry(regKey.data());

			RegistryAccess ra;
			ra.access(regKey);

			Assert::AreEqual(
				cfg.settings.getInterval(), otherCfg.settings.getInterval(),
				L"Reading registry didn't work");
			Assert::AreEqual<UINT>(
				otherCfg.settings.getInterval(), ra.readInt(L"interval"));

			ra.purge();
		}

		TEST_METHOD(TestCfgCommandLineParser)
		{
			Configuration cfg;
			cfg.loadFromCommandLine(L"/I 322 /H 0x0444 /R t.\"\"st");

			Assert::AreEqual<UINT>(322, cfg.settings.getInterval(),
				L"Interval did not match.");
			Assert::AreEqual(0x0444, (int) cfg.settings.getHotkey(),
				L"Hotkey did not match.");
			Assert::IsTrue(cfg.filter.isRegex(),
				L"IsFilterRegex did not match");
			Assert::AreEqual<wstring>(L"t.st", cfg.filter.getFilter(),
				L"Filter did not match");

			Assert::ExpectException<CLIException>([&cfg]()
			{
				cfg.loadFromCommandLine(L"/I 322 /A /H 0x0444");
			}, L"accepted bad arguments");

			Assert::ExpectException<AutoSaveException>([&cfg]() {
				cfg.loadFromCommandLine(L"blargl.exe");
			});

			cfg.loadFromCommandLine(
				LR"(C:\Windows\System32\notepad.exe)");
			Assert::IsTrue(cfg.connection.isConnected(),
				L"did not connect to notepad");
			if (cfg.connection.waitForInputIdle(1000) == WAIT_TIMEOUT)
				Assert::Fail(L"Notepad took too long to open.");
			closeConnectedProcess(cfg);
		}

		TEST_METHOD(TestCfgWindowMatch)
		{
			Configuration connectedCfg, filteredCfg;
			connectedCfg.loadFromCommandLine(
				LR"(C:\Windows\System32\notepad.exe)");
			filteredCfg.filter.setFilter(L" - editor", false);
			if (connectedCfg.connection.waitForInputIdle(1000) == WAIT_TIMEOUT)
				Assert::Fail(L"Notepad took too long to open.");

			Assert::IsTrue(connectedCfg.matchingWindowExists(),
				L"Connected Configuration didn't find window.");
			Assert::IsTrue(filteredCfg.matchingWindowExists(),
				L"Filtered Configuration didn't find window.");
			closeConnectedProcess(connectedCfg);

			Assert::IsFalse(connectedCfg.matchingWindowExists(),
				L"Connected Configuration erroneously found window.");
			Assert::IsFalse(filteredCfg.matchingWindowExists(),
				L"Filtered Configuration erroneously found window.");

			filteredCfg.loadFromCommandLine(
				LR"(C:\Windows\System32\notepad.exe)");
			if (filteredCfg.connection.waitForInputIdle(1000) == WAIT_TIMEOUT)
				Assert::Fail(L"Notepad took too long to open.");

			Assert::IsFalse(connectedCfg.matchingWindowExists(),
				L"Wrong Configuration found window.");
			Assert::IsTrue(filteredCfg.matchingWindowExists(),
				L"Correct Configuration didn't find window.");
			closeConnectedProcess(filteredCfg);
		}
	};
}