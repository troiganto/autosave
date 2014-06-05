#include "stdafx.h"
#include "CppUnitTest.h"
#include "Matcher.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AutoSave_tests
{
	TEST_CLASS(MatcherTests)
	{
	public:
		
		TEST_METHOD(TestMatcherGetFilter)
		{
			Matcher m(L"A", L"B", true);

			Assert::AreEqual<wstring>(L"B", m.getFilter());
			m.useRegex(false);
			Assert::AreEqual<wstring>(L"A", m.getFilter());
		}

		TEST_METHOD(TestMatcherBadRegex)
		{
			wstring badRegex = L"(bad] regex";
			wstring goodRegex = L"(good) regex";
			Matcher m(badRegex, true);

			Assert::IsTrue(m.isRegexBad());
			Assert::IsFalse(m.isValid());
			Assert::IsFalse(m.match(badRegex));

			m.useRegex(false);
			Assert::IsTrue(m.isEmpty());

			m.setPhrase(L"A");
			Assert::IsTrue(m.isValid());
			m.useRegex(true);
			Assert::IsFalse(m.isValid());

			m.setFilter(goodRegex, true);
			Assert::IsTrue(m.isValid());
			Assert::IsFalse(m.isRegexBad());
		}

		TEST_METHOD(TestMatcherMatch)
		{
			Matcher m(L"phrase", L". Reg[[:alnum:]]x", false);
			Assert::IsFalse(m.isRegexBad());

			Assert::IsTrue(m.match(L"An Ordinary Phrase"));
			Assert::IsTrue(m.match(L"IMMA FIRIN MAH PHRASERS"));
			Assert::IsFalse(m.match(L"phras"));
			Assert::IsFalse(m.match(L"hrase"));

			m.useRegex(true);
			Assert::IsTrue(m.isValid());
			Assert::IsFalse(m.match(L". Reg[[:alnum:]]x"));
			Assert::IsFalse(m.match(L". Reg:x"));
			Assert::IsTrue(m.match(L"a regex"));
			Assert::IsTrue(m.match(L"the reg9x"));
			Assert::IsFalse(m.match(L"a reg.x"));

		}

	};
}