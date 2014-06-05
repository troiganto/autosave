#include "stdafx.h"
#include "CppUnitTest.h"
#include "ConnectedShortcut.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AutoSave_tests
{
	TEST_CLASS(ConnectedShortcutTests)
	{
	public:
		
		wstring autosavePath = LR"(C:\dev\autosave\debug\autosave.exe)";
		wstring testfilePath = LR"(C:\dev\autosave\autosave test files\csc\)";
		wstring tempfilePath = testfilePath + L"temp.lnk";

		TEST_METHOD_INITIALIZE(initMethod)
		{
			ConnectedShortcut::setSelfPath(autosavePath);
		}

		TEST_METHOD_CLEANUP(exitMethod)
		{
			DeleteFile(tempfilePath.data());
		}

		TEST_METHOD(TestCSCIsConnected)
		{
			vector<std::pair<wstring, bool>> testData = {
					{ L"broken connected shortcut.lnk", false },
					{ L"file.txt + AutoSave.lnk", true },
					{ L"link.lnk", false },
					{ L"second file.txt + AutoSave.lnk", true },
					{ L"second file.txt", false },
					{ L"non-existing.lnk", false },
					{ L"non-existing.txt", false },
			};
			for (const std::pair<wstring, bool> p : testData)
			{
				Assert::AreEqual<bool>(p.second,
					ConnectedShortcut::isConnected(testfilePath + p.first),
					p.first.data());
			}
		}

		TEST_METHOD(TestCSCConnectDisconnect)
		{
			ConnectedShortcut csc;
			csc.load(testfilePath + L"link.lnk", STGM_READ);
			Assert::IsFalse(csc.isConnected());
			csc.connect(L"");
			Assert::IsTrue(csc.isConnected());
			csc.disconnect();
		}

		TEST_METHOD(TestCSCConnectWorking)
		{
			ConnectedShortcut csc;
			csc.load(testfilePath + L"link.lnk", STGM_READ);
			csc.connect(L"/V 2 /I 10");
			csc.save(tempfilePath, TRUE);
			Assert::AreEqual<wstring>(
				L"AutoSave + link", csc.getDescription());
			Assert::IsTrue(ConnectedShortcut::isConnected(tempfilePath));
		}

		TEST_METHOD(TestCSCConnectWithArguments)
		{
			ConnectedShortcut csc;
			csc.load(testfilePath + L"link.lnk", STGM_READ);
			csc.connect(L"/V 2 /I 10");
			Assert::IsTrue(_tcsicmp(
				(L"/V 2 /I 10 \"" + testfilePath + L"file.txt\"").data(),
				csc.getArguments().data()) == 0);
		}

		TEST_METHOD(TestCSCDisconnectWorking)
		{
			ConnectedShortcut csc;
			csc.load(testfilePath + L"file.txt + AutoSave.lnk", STGM_READ);
			csc.disconnect();
			csc.save(tempfilePath, TRUE);
			Assert::IsTrue(_tcsicmp(
				(testfilePath + L"file.txt").data(),
				csc.getDescription().data()) == 0);
			Assert::IsFalse(ConnectedShortcut::isConnected(tempfilePath));
		}

		TEST_METHOD(TestCSCDoubleConnect)
		{
			ConnectedShortcut changer, examiner;
			changer.load(testfilePath + L"file.txt + AutoSave.lnk", STGM_READ);
			changer.connect(L"");
			changer.save(tempfilePath, TRUE);
			examiner.setSelfPath(autosavePath);
			examiner.load(testfilePath + L"file.txt + AutoSave.lnk", STGM_READ);
			Assert::AreEqual<wstring>(
				changer.getArguments(), examiner.getArguments());
		}

		TEST_METHOD(TestCSCDoubleDisconnect)
		{
			ConnectedShortcut changer, examiner;
			changer.load(testfilePath + L"link.lnk", STGM_READ);
			changer.disconnect();
			changer.save(tempfilePath, TRUE);
			examiner.setSelfPath(autosavePath);
			examiner.load(testfilePath + L"link.lnk", STGM_READ);
			Assert::AreEqual<wstring>(
				changer.getArguments(), examiner.getArguments());
		}

		TEST_METHOD(TestCSCReconnect)
		{
			ConnectedShortcut csc;
			csc.load(testfilePath + L"second file.txt + AutoSave.lnk", STGM_READ);
			csc.reconnect(L"/I 10");
			
			Assert::IsTrue(_tcsicmp(
				(L"/I 10 \"" + testfilePath + L"second file.txt\"").data(),
				csc.getArguments().data()) == 0);
		}

		TEST_METHOD(TestCSCConnectFileName)
		{
			vector<std::pair<wstring, wstring>> testData = {
					{ L"file.txt", L"file.txt + AutoSave.lnk"},
					{ L"link.lnk", L"link + AutoSave.lnk" },
			};
			for (const std::pair<wstring, wstring>& p : testData)
			{
				Assert::AreEqual<wstring>(
					p.second,
					ConnectedShortcut::connectFileName(testfilePath + p.first),
					p.first.data());
			}
		}

		TEST_METHOD(TestCSCDisconnectFileName)
		{
			vector<std::pair<wstring, wstring>> testData = {
					{ L"file.txt + AutoSave.lnk", L"file.txt.lnk" },
					{ L"link + AutoSave.lnk", L"link.lnk" },
					{ L"link + AutoSav.lnk", L"link + AutoSav.lnk" },
					{ L"link + AutoSave.LNK", L"link.lnk" },
			};
			for (const std::pair<wstring, wstring>& p : testData)
			{
				Assert::AreEqual<wstring>(
					p.second,
					ConnectedShortcut::disconnectFileName(p.first),
					p.second.data());
			}
		}
	};
}