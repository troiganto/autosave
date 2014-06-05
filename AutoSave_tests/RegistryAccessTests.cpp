#include "stdafx.h"
#include "CppUnitTest.h"

#include "RegistryAccess.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AutoSave_tests
{
	TEST_CLASS(RegistryAccessTests)
	{
	public:

		TEST_METHOD_INITIALIZE(ClassInit)
		{
			m_ra = new RegistryAccess;
			m_ra->access(L"Broken AutoSave Registry Test");
		}

		TEST_METHOD_CLEANUP(ClassExit)
		{
			m_ra->purge();
			delete m_ra;
		}



		TEST_METHOD(TestRATwoInstancesCanExist)
		{
			try
			{
				RegistryAccess ra;
				ra.access(L"Broken AutoSave Second Registry Test");
				ra.purge();
			}
			catch (RegistryException& exc) {
				Assert::Fail(exc.wcwhat());
			}
		}

		TEST_METHOD(TestRAKeyExists)
		{
			Assert::IsFalse(m_ra->keyExists(L"Some non-existing key"),
				L"Key exists, but shouldn't.");
			Assert::IsFalse(m_ra->keyExists(L"Some non-existing key"),
				L"keyExists created a key!");
			Assert::IsTrue(m_ra->keyExists(m_ra->getKeyName()),
				L"Key doesn't exist, but should.");
		}



		TEST_METHOD(TestRAPurge)
		{
			RegistryAccess ra;
			ra.access(L"Broken AutoSave Second Registry Test");
			ra.purge();
			Assert::IsFalse(ra.isOpen(), L"object open after purging");
			Assert::IsFalse(
				RegistryAccess::keyExists(L"Broken AutoSave Second Registry Test"),
				L"Purge didn't delete key");
		}

		TEST_METHOD(TestRAHasKeyBeenCreated)
		{
			Assert::IsTrue(m_ra->hasKeyBeenCreated(),
				L"created new key but didn't realize it");
			RegistryAccess ra;
			ra.access(L"Broken AutoSave Registry Test");
			Assert::IsFalse(ra.hasKeyBeenCreated(),
				L"didn't create a new key but claimed to have done so");
			ra.close();
		}

		TEST_METHOD(TestRAWrite)
		{
			try {
				m_ra->writeInt(L"Test int", 42);
				m_ra->writeString(L"Test string", L"Rainbow Dash");
			}
			catch (RegistryException& exc) {
				Assert::Fail(exc.wcwhat());
			}
		}

		TEST_METHOD(TestRARead)
		{
			m_ra->writeInt(L"Test int", 42);
			m_ra->writeString(L"Test string", L"Rainbow Dash");

			Assert::AreEqual(42, m_ra->readInt(L"Test int"));
			Assert::AreEqual(
				L"Rainbow Dash",
				m_ra->readString(L"Test string").data());

			Assert::ExpectException<RegistryException>([this]()
			{
				m_ra->readInt(L"Test string");
			}, L"reading a string, but expecting an int");
			Assert::ExpectException<RegistryException>([this]()
			{
				m_ra->readMultiString(L"Test string");
			}, L"reading a string, but expecting a multistring");
			Assert::ExpectException<RegistryException>([this]()
			{
				m_ra->readString(L"Test int");
			}, L"reading an int, but expecting a string");
		}

		TEST_METHOD(TestRAReadNonExistingValues)
		{
			try
			{
				m_ra->readInt(L"non-existing entry");
				Assert::Fail(L"Expected RegistryError, but there was none.");
			}
			catch (RegistryException& exc) {
				if (exc.errorCode() != ERROR_FILE_NOT_FOUND)
				{
					Logger::WriteMessage(exc.wcwhat());
					throw;
				}
			}
		}

		TEST_METHOD(TestRAMultistrings)
		{
			RegistryAccess* pra = m_ra;
			Assert::ExpectException<RegistryException>([pra]()
			{
				pra->readMultiString(L"non-existing entry");
			}, L"reading non-existing multistring");
			
			const vector<wstring> sample(2, L"nyeh");

			m_ra->writeMultiString(L"writereadtest", sample);
			Assert::IsTrue(
				sample == m_ra->readMultiString(L"writereadtest"),
				L"read/write multistring");

			const wstring single = L"More stuff to test!";
			m_ra->AppendToMultiString(L"writereadtest", single);
			Assert::AreEqual<size_t>(3, m_ra->readMultiString(L"writereadtest").size(),
				L"append multistring size check");
			Assert::AreEqual<>(single, m_ra->readMultiString(L"writereadtest")[2],
				L"append multistring content check");

			m_ra->AppendToMultiString(L"appendtoempty", single);
			Assert::AreEqual<size_t>(1, m_ra->readMultiString(L"appendtoempty").size(),
				L"append to empty value size check");
			Assert::AreEqual(single, m_ra->readMultiString(L"appendtoempty")[0],
				L"append to empty value content check");
		}

		wstring ansiToWide(LPCSTR ansi)
		{
			if (ansi == "")
				return L"No error message specified";

			size_t sizeNeeded = MultiByteToWideChar(
				CP_ACP, MB_USEGLYPHCHARS, ansi, -1, NULL, 0);
			if (sizeNeeded == 0)
				return L"Couldn't retrieve error message";

			std::auto_ptr<wchar_t> wide(new wchar_t[sizeNeeded]);
			size_t result = MultiByteToWideChar(
				CP_ACP, MB_USEGLYPHCHARS, ansi, -1, wide.get(), (int)sizeNeeded);
			if (result == 0)
				return L"Couldn't retrieve error message";

			return wide.get();
		}

		TEST_METHOD(TestRAReadKeyDefaultString)
		{
			Assert::AreEqual<wstring>(
				L"txtfile",
				RegistryAccess::readKeyDefaultString(HKEY_CLASSES_ROOT, L".txt"));
			Assert::AreEqual<wstring>(
				LR"("C:\Program Files (x86)\VideoLAN\VLC\vlc.exe",0)",
				RegistryAccess::readKeyDefaultString(
				HKEY_CLASSES_ROOT, L"VLC.ogg\\DefaultIcon"));
			Assert::AreEqual<wstring>(
				L"",
				RegistryAccess::readKeyDefaultString(
				HKEY_CLASSES_ROOT, L".sai\\DefaultIcon\\non-existing"));
			try {
				RegistryAccess::readKeyDefaultString(
					(HKEY) 42, L"parent key does not exist");
				Assert::Fail();
			}
			catch (AutoSaveException& exc) {
				if (exc.errorCode() != ERROR_INVALID_HANDLE)
					throw;
			}
		}

	private:
		RegistryAccess* m_ra = NULL;

	};
}