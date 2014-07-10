#include "stdafx.h"
#include "CppUnitTest.h"
#include "TemporaryShortcutFile.h"
#include "ConnectedShortcut.h"
#include "MiscSettings.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AutoSave_tests
{
	TEST_CLASS(TemporaryShortcutFileTests)
	{
	public:
		
		wstring notepad = LR"(C:\Windows\System32\notepad.exe)";
		wstring dir = LR"(C:\dev\autosave\autosave test files\tsf)";
		wstring link = dir + L"\\link.lnk";
		wstring sf1 = dir + L"\\file.txt";
		wstring sf2 = dir + L"\\second file.txt";
		wstring cs1 = dir + L"\\file.txt + AutoSave.lnk";
		wstring cs2 = dir + L"\\second file.txt + AutoSave.lnk";

		TEST_METHOD(TestTSFNull)
		{
			TemporaryShortcutFile tsf;
			Assert::IsTrue(tsf.isNull());
			Assert::AreEqual<wstring>(L"", tsf.getFile());
		}

		TEST_METHOD(TestTSFUpdateSettingsWithoutFile)
		{
			MiscSettings ms;
			TemporaryShortcutFile tsf;
			tsf.setSaveDirectory(dir);
			tsf.updateSettings(ms, MiscSettings::ATT_ALL);
		}

		TEST_METHOD(TestTSFDestructor)
		{
			{
				TemporaryShortcutFile tsf;
				tsf.setSaveDirectory(dir);
				tsf.updateFile(sf1);
				// Make sure the file exists after the call.
				Assert::IsTrue(PathFileExists(cs1.data()) != FALSE);
			}
			// Make sure it doesn't exist anzmore after deleting tsf.
			Assert::IsFalse(PathFileExists(cs1.data()) != FALSE);
		}

		TEST_METHOD(TestTSFConnecting)
		{
			TemporaryShortcutFile tsf;
			tsf.setSaveDirectory(dir);

			tsf.updateFile(sf1);
			Assert::AreEqual<wstring>(tsf.getFile(), cs1);
			Assert::IsTrue(PathFileExists(cs1.data()) != FALSE);

			tsf.updateFile(sf2);
			Assert::AreEqual<wstring>(tsf.getFile(), cs2);
			Assert::IsFalse(PathFileExists(cs1.data()) != FALSE);
			Assert::IsTrue(PathFileExists(cs2.data()) != FALSE);

			MiscSettings ms;
			tsf.updateSettings(ms, MiscSettings::ATT_VERBOSITY);
			wstring args = tsf.getArguments();
			Assert::IsTrue(_tcsicmp(
				(L"/V 2 \"" + sf2 + L"\"").data(), args.data()) == 0);

			Assert::IsTrue(ConnectedShortcut::isConnected(cs2));
		}

		TEST_METHOD(TestTSFShortcutProperties)
		{
			TemporaryShortcutFile tsf;
			tsf.setSaveDirectory(dir);

			tsf.updateFile(sf1);
			Assert::IsTrue(0 == _tcsicmp(
				tsf.getSelfPath().data(), tsf.getPath().data()));
			Assert::IsTrue(0 == _tcsicmp(
				(L"\"" + sf1 + L"\"").data(), tsf.getArguments().data()));
			Assert::IsTrue(0 == _tcsicmp(
				dir.data(), tsf.getWorkingDirectory().data()));
		}

		TEST_METHOD(TestTSFTemporaryRepairedShortcut)
		{
			wstring repaired;
			try {
				TemporaryRepairedShortcut trs;
				repaired = trs.repair(link);
			}
			catch (OleException& exc) {
				Assert::Fail(exc.wcwhat());
			}

			Logger::WriteMessage(repaired.data());
			Assert::AreEqual<wstring>(
				repaired.substr(repaired.size() - 8),
				L"link.lnk");
		}
	};
}