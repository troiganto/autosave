#include "stdafx.h"
#include "CppUnitTest.h"
#include "MiscSettings.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft {
	namespace VisualStudio {
		namespace CppUnitTestFramework
		{
			template<>
			static std::wstring
				ToString<MiscSettings::Verbosity>(const MiscSettings::Verbosity& v)
			{
				return ToString<int>((int) v);
			}
		}
	}
}

namespace AutoSave_tests
{
	TEST_CLASS(MiscSettingsTest)
	{
	public:
		
		TEST_METHOD(TestMSGettersAndSetters)
		{
			MiscSettings ms;

			ms.setInterval(20);
			Assert::AreEqual<int>(20, ms.getInterval());

			ms.setVerbosity(MiscSettings::Verbosity::SHOW_ICONS);
			Assert::AreEqual<MiscSettings::Verbosity>(
				MiscSettings::Verbosity::SHOW_ICONS,
				ms.getVerbosity());

			ms.setHotkey(0x0199);
			Assert::AreEqual<int>(0x0199,
				ms.getHotkey());
		}

		TEST_METHOD(TestMSValueBounds)
		{
			MiscSettings ms;

			ms.setInterval(1);
			Assert::AreEqual(ms.getMinInterval(), ms.getInterval(),
				L"Interval min bounding doesn't work");
			ms.setInterval(999999);
			Assert::AreEqual(ms.getMaxInterval(), ms.getInterval(),
				L"Interval max bounding doesn't work");

			ms.setVerbosity(-2);
			Assert::AreEqual<MiscSettings::Verbosity>(
				MiscSettings::MIN_VERBOSITY, ms.getVerbosity(),
				L"Verbosity min bounding doesn't work");
			ms.setVerbosity(999999);
			Assert::AreEqual<MiscSettings::Verbosity>(
				MiscSettings::MAX_VERBOSITY, ms.getVerbosity(),
				L"Verbosity max bounding doesn't work");

			ms.setHotkey(MAXWORD);
			Assert::AreEqual<int>(0xFFFF, ms.getHotkey(),
				L"Hotkey max bounding doesn't work");

		}

		TEST_METHOD(TestMSToCommandLine)
		{
			MiscSettings ms;
			ms.setInterval(666);
			ms.setHotkey(0x0444);
			ms.setVerbosity(MiscSettings::MAX_VERBOSITY);

			Assert::AreEqual<wstring>(
				L"", ms.toCommandLine(MiscSettings::ATT_NONE));
			Assert::AreEqual<wstring>(
				L"/I 666 ", ms.toCommandLine(MiscSettings::ATT_INTERVAL));
			Assert::AreEqual<wstring>(
				L"/H 0x0444 ", ms.toCommandLine(MiscSettings::ATT_HOTKEY));
			Assert::AreEqual<wstring>(
				L"/V 3 ", ms.toCommandLine(MiscSettings::ATT_VERBOSITY));

			Assert::AreEqual<wstring>(
				L"/I 666 /H 0x0444 /V 3 ",
				ms.toCommandLine(MiscSettings::ATT_ALL));
		}
		
		TEST_METHOD(TestMSMaxCommandLineLength)
		{
			MiscSettings ms;
			ms.setInterval(MAXUINT);
			ms.setHotkey(MAXWORD);
			ms.setVerbosity(MiscSettings::MAX_VERBOSITY);
			Assert::AreEqual<wstring>(
				L"/I 86400 /H 0xffff /V 3 ",
				ms.toCommandLine(MiscSettings::ATT_ALL));
		}
	};
}