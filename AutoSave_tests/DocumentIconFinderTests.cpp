#include "stdafx.h"
#include "CppUnitTest.h"
#include "DocumentIconFinder.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AutoSave_tests
{
	TEST_CLASS(DocumentIconFinderTests)
	{
	public:

		wstring imageres = LR"(C:\Windows\system32\imageres.dll)";
		wstring pngapp = LR"(C:\Program Files (x86)\IrfanView\i_view32.exe)";
		wstring icofile = LR"(C:\dev\autosave\AutoSave Icons\main.ico)";
		wstring saiapp = LR"(C:\Users\Nico\Downloads\PaintToolSAI\sai.exe)";
		wstring oggapp = LR"(C:\Program Files (x86)\VideoLAN\VLC\vlc.exe)";
#ifndef _WIN64
		wstring self = LR"(C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO 12.0\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\TESTWINDOW\vstest.executionengine.x86.exe)";
#else
		wstring self = LR"(C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO 12.0\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\TESTWINDOW\vstest.executionengine.exe)";
#endif

		TEST_METHOD(TestDIFTextDocuments)
		{
			DocumentIconFinder dif(L"test.txt");
			Assert::AreEqual<wstring>(imageres,
				dif.iconFile, L"didn't find TXT icon file");
			Assert::AreEqual(-102, dif.iconIndex, L"didn't find TXT icon index");
		}

		TEST_METHOD(TestDIFPNGFiles)
		{
			DocumentIconFinder dif(L"test.png");
			// To be replaced with yout own picture viewer's icon.
			Assert::AreEqual<wstring>(
				pngapp, dif.iconFile, L"didn't find PNG icon file");
			Assert::AreEqual(0, dif.iconIndex, L"didn't find PNG icon index");
		}

		TEST_METHOD(TestDIFIconFiles)
		{
			DocumentIconFinder dif(icofile);
			Assert::AreEqual<wstring>(icofile, dif.iconFile,
				L"didn't find ICO icon file (itself)");
			Assert::AreEqual(0, dif.iconIndex, L"didn't find icon file index (0)");
		}

		TEST_METHOD(TestDIFSaiFiles)
		{
			// Tests the unusual .sai\DefaultIcon structure.
			DocumentIconFinder dif(L"test.sai");
			Assert::AreEqual<wstring>(
				LR"(C:\Users\Nico\Downloads\PaintToolSAI\sai.exe)",
				dif.iconFile, L"didn't find SAI icon file");
			Assert::AreEqual(1, dif.iconIndex, L"didn't find SAI icon index");
		}

		TEST_METHOD(TestDIFExeFiles)
		{
			// Tests the unusual .sai\DefaultIcon structure.
			DocumentIconFinder dif(saiapp);
			Assert::AreEqual<wstring>(
				saiapp, dif.iconFile, L"didn't find EXE icon file (itself)");
			Assert::AreEqual(0, dif.iconIndex, L"didn't find EXE icon index (0)");
		}

		TEST_METHOD(TestDIFOggFiles)
		{
			// Tests quotes in the icon location string.
			DocumentIconFinder dif(L"test.ogg");
			Assert::AreEqual<wstring>(
				oggapp, dif.iconFile, L"didn't find OGG icon file");
			Assert::AreEqual(0, dif.iconIndex, L"didn't find OGG icon index");
		}

		TEST_METHOD(TestDIFDefaultIcon)
		{
			DocumentIconFinder dif(L"blah");
			Assert::AreEqual<wstring>(self, dif.iconFile,
				L"didn't find default icon file");
			Assert::AreEqual(-101, dif.iconIndex,
				L"didn't find default icon index (has it changed?)");
		}

	};
}