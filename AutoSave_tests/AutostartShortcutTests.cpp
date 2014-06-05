#include "stdafx.h"
#include "CppUnitTest.h"
#include "AutostartShortcut.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AutoSave_tests
{
	static wstring filePath;

	static bool fileExists()
	{
		return PathFileExists(filePath.data()) != FALSE;
	}

	TEST_CLASS(AutostartShortcutTests)
	{
	public:
		
		TEST_CLASS_INITIALIZE(InitClass)
		{
			filePath = AutostartShortcut::locate();
			AutostartShortcut::remove();
		}

		TEST_CLASS_CLEANUP(ExitClass)
		{
			// We cannot recreate the autostart shortcut --
			// OleUtils::getSelfPath returns the wrong executable!
		}

		TEST_METHOD(TestASSEmptyDelete)
		{
			AutostartShortcut::remove();
		}

		TEST_METHOD(TestASSCreateAndRemove)
		{
			Assert::IsFalse(fileExists());
			AutostartShortcut::create();
			Assert::IsTrue(fileExists());
			AutostartShortcut::remove();
			Assert::IsFalse(fileExists());
		}

		TEST_METHOD(TestASSCorrectlyCreated)
		{
			AutostartShortcut::create();
			Shortcut sc;
			sc.load(filePath, STGM_READ);
			Assert::IsTrue(OleUtils::isSelf(sc.getPath()));
			Assert::AreEqual<wstring>(L"", sc.getArguments());
			AutostartShortcut::remove();
		}

		TEST_METHOD(TestASSDoubleCreate)
		{
			Assert::IsFalse(fileExists());
			AutostartShortcut::create();
			Assert::IsTrue(fileExists());
			AutostartShortcut::create();
			Assert::IsTrue(fileExists());
			AutostartShortcut::remove();
			Assert::IsFalse(fileExists());
		}

	};
}