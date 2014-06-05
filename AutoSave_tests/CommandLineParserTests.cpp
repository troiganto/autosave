#include "stdafx.h"
#include "CppUnitTest.h"
#include "CommandLineParser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace AutoSave_tests
{
	TEST_CLASS(CommandLineParserTests)
	{
	public:
		
		TEST_METHOD(TestCLIConstructor)
		{
			CommandLineParser cli;
			Assert::IsTrue(cli.getLArgsString().empty());
			Assert::IsTrue(cli.getLArgs().empty());

			cli.parse(L"");
			Assert::IsTrue(cli.getLArgsString().empty());
			Assert::IsTrue(cli.getLArgs().empty());
		}

		TEST_METHOD(TestCLIEscapeArguments)
		{
			wstring result, msg;
			unordered_map<wstring, wstring> test_pairs = {
					{ LR"(C:\dev\autosave\LICENSE)", LR"(C:\dev\autosave\LICENSE)" },
					{ LR"(C:\dev\auto save\LICENSE)", LR"("C:\dev\auto save\LICENSE")" },
					{ LR"(this is a "cool" test)", LR"("this is a \"cool\" test")" },
					{ LR"(\\test\\")", LR"(\\test\\\\\")" },
			};
			for (pair<wstring, wstring> test_pair : test_pairs)
			{
				result = CommandLineParser::escapeArgument(test_pair.first);
				msg = L"Error: Expected " + test_pair.second +
					L", but got " + result;
				Assert::IsTrue(result == test_pair.second, msg.data());
			}
		}

		TEST_METHOD(TestCLIParse)
		{
			CommandLineParser cli;
			cli.setAllowedKeys(L"IHF");

			cli.parse(L"par1 \"par 2\" par3 par4");
			Assert::IsTrue(cli.getLArgsString() == L"par1 \"par 2\" par3 par4",
				L"larg test");

			Assert::ExpectException<CLIException>([&cli]()
			{
				cli.parse(L"/invalid 10");
			}, L"wrong length test failed");

			Assert::ExpectException<CLIException>([&cli]()
			{
				cli.parse(L"/X 10");
			}, L"wrong letter test failed");

			Assert::ExpectException<CLIException>([&cli]()
			{
				cli.parse(L"/I 10 /H");
			}, L"missing option test failed");

			cli.parse(L"/I 10 /H 0x4411");
			Assert::IsTrue(cli.getLArgs().empty(), L"empty largs test");
		}

		TEST_METHOD(TestCLIGetters)
		{
			CommandLineParser cli;
			cli.setAllowedKeys(L"IHF");
			cli.parse(L"/I 10 /H 0x0153 /F test te\\\"st test test");

			Assert::IsTrue(cli.kwArgsContain(L'I'), L"contain I");
			Assert::IsTrue(cli.kwArgsContain(L'H'), L"contain H");
			Assert::IsFalse(cli.kwArgsContain(L'X'), L"no contain X");

			Assert::AreEqual(cli.getIntKwArg(L'I'), 10, L"value I");
			Assert::AreEqual(cli.getIntKwArg(L'H'), 0x0153, L"value H");
			Assert::IsTrue(cli.getStringKwArg(L'F') == L"test", L"value F");
			Assert::IsTrue(cli.getLArgsString() == L"te\\\"st test test",
				L"larg test");

			Assert::ExpectException<invalid_argument>([&cli]()
			{
				cli.getIntKwArg(L'F');
			}, L"get int from string test");

			Assert::ExpectException<out_of_range>([&cli]()
			{
				cli.getStringKwArg(L'X');
			}, L"get non-existing option test");
		}

		TEST_METHOD(TestCLIJoinArguments)
		{
			CommandLineParser cli;
			vector<wstring> inputs = {
				L"test",
				L"test test",
				L"\"test test\" test",
				L"test \\\"test test \\\"test"
			};
			wstring errorMsg;
			for (const wstring& input : inputs)
			{
				cli.parse(input);
				vector<wstring> args = cli.getLArgs();
				const wstring output = CommandLineParser::joinArguments(args);
				errorMsg = L"Assert failed at string: " + input;
				Assert::AreEqual<wstring>(input, output, errorMsg.data());
			}
		}

	};
}