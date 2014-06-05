#include "stdafx.h"
#include "CppUnitTest.h"
#include "AutoSaveException.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace AutoSave_tests
{
	TEST_CLASS(AutoSaveExceptionTests)
	{
	public:
		
		TEST_METHOD(TestASENeutralException)
		{
			AutoSaveException exc(S_OK);
			Assert::IsTrue(SUCCEEDED(exc.hResult()));
		}

		TEST_METHOD(TestASEThrowOnFailure)
		{
			throwOnFailure<AutoSaveException>(S_OK);
			throwOnFailure<AutoSaveException>(DRAGDROP_S_DROP);
			Assert::ExpectException<AutoSaveException>([]() {
				throwOnFailure<AutoSaveException>(E_FAIL);
			});
		}

		TEST_METHOD(TestASEThrowIfZero)
		{
			Assert::ExpectException<AutoSaveException>([]() {
				throwIfZero<AutoSaveException>(0);
			});
			throwIfZero<AutoSaveException>(3);
			Assert::AreEqual<int>(33, throwIfZero<AutoSaveException, int>(33));
		}

		TEST_METHOD(TestASEDefaultConstructor)
		{
			long errorCode = ERROR_FILE_NOT_FOUND;
			try {
				SetLastError(errorCode);
				throwIfZero<AutoSaveException>(0);
				Assert::Fail();
			}
			catch (AutoSaveException& exc) {
				Assert::AreEqual(errorCode, exc.errorCode());
				Assert::AreEqual<HRESULT>(0x80070002, exc.hResult());
			}
		}

		TEST_METHOD(TestASECastConstructor)
		{
			HRESULT hres = E_ACCESSDENIED;
			try {
				throwOnFailure<AutoSaveException>(hres);
				Assert::Fail();
			}
			catch (AutoSaveException& exc) {
				Assert::AreEqual(hres, exc.hResult());
				Assert::AreEqual(ERROR_ACCESS_DENIED, exc.errorCode());
			}
		}

		TEST_METHOD(TestASEWideText)
		{
			// Must be adapted to your choide of language.
			vector<pair<LPCWSTR, AutoSaveException>> pairs{
				{ L"Der Vorgang wurde erfolgreich beendet.\r\n", S_OK },
				{ L"Unbekannter Fehler\r\n", E_FAIL },
				{ L"Das System kann die angegebene Datei nicht finden.\r\n", ERROR_FILE_NOT_FOUND },
				{ L"Unknown exception (HRESULT value 0xDEADBEEF)\r\n", 0xDEADBEEF },
			};
			Assert::AreEqual<size_t>(4, pairs.size());
			for (pair<LPCWSTR, AutoSaveException>& p : pairs)
			{
				Assert::AreEqual<wstring>(p.first, p.second.wcwhat());
			}
		}

		TEST_METHOD(TestASEAnsiText)
		{
			// Must be adapted to your choice of language.
			vector<pair<LPCSTR, AutoSaveException>> pairs{
				{ "Der Vorgang wurde erfolgreich beendet.\r\n", S_OK },
				{ "Unbekannter Fehler\r\n", E_FAIL },
				{ "Das System kann die angegebene Datei nicht finden.\r\n", ERROR_FILE_NOT_FOUND },
				{ "Unknown exception (HRESULT value 0xDEADBEEF)\r\n", 0xDEADBEEF },
			};
			Assert::AreEqual<size_t>(4, pairs.size());
			for (pair<LPCSTR, AutoSaveException>& p : pairs)
			{
				Assert::AreEqual<string>(p.first, p.second.what());
			}
		}
	};
}