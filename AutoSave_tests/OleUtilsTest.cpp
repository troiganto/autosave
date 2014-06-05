#include "stdafx.h"
#include "CppUnitTest.h"
#include "OleUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace AutoSave_tests
{
	TEST_CLASS(OleUtilsTest)
	{
	public:
		
#ifndef _WIN64
		const wstring self = LR"(C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO 12.0\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\TESTWINDOW\vstest.executionengine.x86.exe)";
		const wstring selfAllLowers = LR"(c:\program files (x86)\microsoft visual studio 12.0\common7\ide\commonextensions\microsoft\testwindow\vstest.executionengine.x86.exe)";
#else
		const wstring self = LR"(C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO 12.0\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\TESTWINDOW\vstest.executionengine.exe)";
		const wstring selfAllLowers = LR"(c:\program files (x86)\microsoft visual studio 12.0\common7\ide\commonextensions\microsoft\testwindow\vstest.executionengine.exe)";
#endif
		const wstring file = LR"(C:\dev\autosave\autosave test files\ole\file.txt)";
		const wstring link = LR"(C:\dev\autosave\autosave test files\ole\link.lnk)";
		const wstring notepad = LR"(C:\windows\system32\notepad.exe)";
		const wstring windows = LR"(C:\windows\)";
		const wstring nonexisting = LR"(C:\non.existing)";

		TEST_METHOD(TestOleGetSelfPath)
		{
			// You will have to modify the expected result.
			wstring expected = LR"(C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO 12.0\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\TESTWINDOW\vstest.executionengine.x86.exe)";
			wstring expectedAllLowers = LR"(c:\program files (x86)\microsoft visual studio 12.0\common7\ide\commonextensions\microsoft\testwindow\vstest.executionengine.x86.exe)";
			Assert::AreEqual<wstring>(self, OleUtils::getSelfPath());
			Assert::IsTrue(OleUtils::isSelf(selfAllLowers));
		}

		TEST_METHOD(TestOleFileRecognition)
		{
			Assert::IsTrue(OleUtils::isExecutable(notepad));
			Assert::IsFalse(OleUtils::isShortcutFile(notepad));

			Assert::IsTrue(OleUtils::isShortcutFile(link));
			Assert::IsFalse(OleUtils::isExecutable(link));

			Assert::IsFalse(OleUtils::isShortcutFile(file));
			Assert::IsFalse(OleUtils::isExecutable(file));
		}

		TEST_METHOD(TestOleShellItemComputer)
		{
			// Change to match your language.
			wstring expectedName = L"Computer";
			wstring itemName;
			IShellItem& item = OleUtils::getFileShellItem(L"");
			{
				LPWSTR buffer;
				item.GetDisplayName(SIGDN_NORMALDISPLAY, &buffer);
				itemName = buffer;
				CoTaskMemFree(buffer);
			}
			Assert::AreEqual<wstring>(expectedName, itemName);

			Assert::ExpectException<OleException>([&item]() {
				LPWSTR buffer;
				throwOnFailure<OleException>(
					item.GetDisplayName(SIGDN_FILESYSPATH, &buffer));
				Logger::WriteMessage(buffer);
				CoTaskMemFree(buffer);
			});
			item.Release();
		}

		TEST_METHOD(TestOleShellItemNonExistingFile)
		{
			try {
				IShellItem& item = OleUtils::getFileShellItem(nonexisting);
				item.Release();
				Assert::Fail();
			}
			catch (OleException& exc)
			{
				Assert::AreEqual<DWORD>(ERROR_FILE_NOT_FOUND, exc.errorCode());
			}
		}

		TEST_METHOD(TestOleShellItemFolderAndFile)
		{
			vector<wstring> files = { windows, notepad, file };
			for (const wstring& f : files)
			{
				Logger::WriteMessage((L"Trying \"" + f + L"\"\n").data());
				IShellItem& item = OleUtils::getFileShellItem(f);
				item.Release();
			}
		}

		TEST_METHOD(TestOleDataObject)
		{
			try {
				IDataObject& data = OleUtils::getFileDataObject(L"");
				data.Release();
				Assert::Fail();
			}
			catch (OleException& exc)
			{
				Assert::AreEqual<HRESULT>(E_INVALIDARG, exc.hResult());
			}

			try {
				IDataObject& data = OleUtils::getFileDataObject(nonexisting);
				data.Release();
				Assert::Fail();
			}
			catch (OleException& exc)
			{
				Assert::AreEqual<HRESULT>(ERROR_FILE_NOT_FOUND, exc.errorCode());
			}

			vector<wstring> files = { windows, notepad, file };
			for (const wstring& f : files)
			{
				Logger::WriteMessage((L"Trying \"" + f + L"\"\n").data());
				IDataObject& data = OleUtils::getFileDataObjectWithIcon(f);
				data.Release();
			}
		}

		TEST_METHOD(TestOleUnpackFileDataObject)
		{
			IDataObject& data = OleUtils::getFileDataObject(file);

			vector<wstring> files = OleUtils::UnpackFileDataObject(data);
			Assert::AreEqual<size_t>(1, files.size());
			Assert::IsTrue(_tcsicmp(file.data(), files[0].data()) == 0);
			data.Release();
		}

	};
}