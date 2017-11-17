#include <CppUnitTest.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define FPL_IMPLEMENTATION
#define FPL_NO_WINDOW
#define FPL_NO_VIDEO_OPENGL
#define FPL_AUTO_NAMESPACE
#include <final_platform_layer.hpp>

#include <varargs.h>
#include <string>

namespace Microsoft {
	namespace VisualStudio {
		namespace CppUnitTestFramework {
			// @NOTE(final): Stupid MSVC, no int64_t ToString conversion function built-in O_o
			template<>
			static std::wstring ToString<int64_t>(const int64_t &t) {
				RETURN_WIDE_STRING(t);
			}
		}
	}
}

namespace fpl_tests {

	TEST_CLASS(TestFPLWin32) {
private:
	void Msg(const char *format, ...) {
		char buffer[1024];
		va_list vaList;
		va_start(vaList, format);
		vsprintf_s(buffer, 1024, format, vaList);
		va_end(vaList);
		Logger::WriteMessage(buffer);
	}
public:

	TEST_METHOD(TestCore) {
		Msg("Test InitPlatform with all init flags\n");
		{
			bool result = InitPlatform(InitFlags::All);
			Assert::IsTrue(result, L"", LINE_INFO());
			const char *errorStr = GetPlatformLastError();
			Assert::AreEqual(nullptr, errorStr, L"", LINE_INFO());
			ReleasePlatform();
		}

		Msg("Test InitPlatform with none init flags\n");
		{
			bool result = InitPlatform(InitFlags::None);
			Assert::IsTrue(result, L"", LINE_INFO());
			const char *errorStr = GetPlatformLastError();
			Assert::AreEqual(nullptr, errorStr, L"", LINE_INFO());
			ReleasePlatform();
		}

	#if 0
		Msg("Test InitPlatform with broken global win32 state\n");
		{
			globalWin32State_Internal = (Win32State_Internal *)(void *)1;
			bool result = InitPlatform(InitFlags::None);
			Assert::IsFalse(result);
			// @NOTE(final): We created an invalid global win32 state, so it will fail before allocating the error buffer.
			size_t errorCount = GetPlatformLastErrorCount();
			Assert::AreEqual((size_t)0, errorCount, L"", LINE_INFO());
			const char *errorStr = GetPlatformLastError();
			Assert::AreEqual(nullptr, errorStr, L"", LINE_INFO());
			// @NOTE(final): This will crash because the memory is corrupted!
			ReleasePlatform();
		}
	#endif
	}

	TEST_METHOD(TestMacros) {
		//
		// FPL_ARRAYCOUNT
		//
		Msg("[FPL_ARRAYCOUNT] Test static char array\n");
		{
			char staticArray[137] = {};
			Assert::AreEqual((int)FPL_ARRAYCOUNT(staticArray), 137, L"", LINE_INFO());
			Assert::AreEqual(FPL_ARRAYCOUNT(staticArray), (size_t)137, L"", LINE_INFO());
		}
		Msg("[FPL_ARRAYCOUNT] Test static int array\n");
		{
			int staticArray[349] = {};
			Assert::AreEqual(349, (int)FPL_ARRAYCOUNT(staticArray), L"", LINE_INFO());
			Assert::AreEqual((size_t)349, FPL_ARRAYCOUNT(staticArray), L"", LINE_INFO());
		}
		Msg("[FPL_ARRAYCOUNT] Test static bool array\n");
		{
			bool staticArray[961] = {};
			Assert::AreEqual(961, (int)FPL_ARRAYCOUNT(staticArray), L"", LINE_INFO());
			Assert::AreEqual((size_t)961, FPL_ARRAYCOUNT(staticArray), L"", LINE_INFO());
		}
		Msg("[FPL_ARRAYCOUNT] Test static void pointer array\n");
		{
			void *staticArray[35] = {};
			Assert::AreEqual(35, (int)FPL_ARRAYCOUNT(staticArray), L"", LINE_INFO());
			Assert::AreEqual((size_t)35, FPL_ARRAYCOUNT(staticArray), L"", LINE_INFO());
		}

		// @NOTE(final): This is a simple/stupid macro, so when you pass a pointer, you basically get 2 always
		Msg("[FPL_ARRAYCOUNT] Test nullptr\n");
		{
			int *emptyArray = nullptr;
			Assert::AreEqual(sizeof(int *) / sizeof(int), FPL_ARRAYCOUNT(emptyArray), L"", LINE_INFO());
		}

		Msg("[FPL_ARRAYCOUNT] Test pointer from references static array\n");
		{
			int staticArray[3] = {};
			int *refArray = &staticArray[0];
			Assert::AreEqual(sizeof(int *) / sizeof(int), FPL_ARRAYCOUNT(refArray), L"", LINE_INFO());
		}

		//
		// FPL_OFFSETOF
		//
		Msg("[FPL_OFFSETOF] Test alignment of 4 (High to low)\n");
		{
		#pragma pack(push, 4)
			struct TestStruct {
				uint64_t a;
				uint32_t b;
				uint16_t c;
				uint8_t d;
			};
		#pragma pack(pop)
			Assert::AreEqual((size_t)0, FPL_OFFSETOF(TestStruct, a), L"", LINE_INFO());
			Assert::AreEqual((size_t)8, FPL_OFFSETOF(TestStruct, b), L"", LINE_INFO());
			Assert::AreEqual((size_t)12, FPL_OFFSETOF(TestStruct, c), L"", LINE_INFO());
			Assert::AreEqual((size_t)14, FPL_OFFSETOF(TestStruct, d), L"", LINE_INFO());
		}

		Msg("[FPL_OFFSETOF] Test alignment of 4 (Low to High)\n");
		{
		#pragma pack(push, 4)
			struct TestStruct {
				uint8_t a;
				uint16_t b;
				uint32_t c;
				uint64_t d;
			};
		#pragma pack(pop)
			Assert::AreEqual((size_t)0, FPL_OFFSETOF(TestStruct, a), L"", LINE_INFO());
			Assert::AreEqual((size_t)2, FPL_OFFSETOF(TestStruct, b), L"", LINE_INFO());
			Assert::AreEqual((size_t)4, FPL_OFFSETOF(TestStruct, c), L"", LINE_INFO());
			Assert::AreEqual((size_t)8, FPL_OFFSETOF(TestStruct, d), L"", LINE_INFO());
		}

		Msg("[FPL_OFFSETOF] Test alignment of 8 (Low to High)\n");
		{
		#pragma pack(push, 8)
			struct TestStruct {
				uint8_t a;
				uint16_t b;
				uint8_t c[3];
				uint64_t d;
			};
		#pragma pack(pop)
			Assert::AreEqual((size_t)0, FPL_OFFSETOF(TestStruct, a), L"", LINE_INFO());
			Assert::AreEqual((size_t)2, FPL_OFFSETOF(TestStruct, b), L"", LINE_INFO());
			Assert::AreEqual((size_t)4, FPL_OFFSETOF(TestStruct, c), L"", LINE_INFO());
			Assert::AreEqual((size_t)8, FPL_OFFSETOF(TestStruct, d), L"", LINE_INFO());
		}

		//
		// FPL_MIN/FPL_MAX
		//
		Msg("[FPL_MIN] Test integers\n");
		{
			Assert::AreEqual(3, FPL_MIN(3, 7), L"", LINE_INFO());
			Assert::AreEqual(3, FPL_MIN(7, 3), L"", LINE_INFO());
			Assert::AreEqual(-7, FPL_MIN(-7, -3), L"", LINE_INFO());
			Assert::AreEqual(-7, FPL_MIN(-3, -7), L"", LINE_INFO());
			struct TestStruct {
				int a;
				int b;
			};
			TestStruct instance = { 3, 7 };
			TestStruct *instancePtr = &instance;
			Assert::AreEqual(3, FPL_MIN(instancePtr->a, instancePtr->b), L"", LINE_INFO());
		}
		Msg("[FPL_MIN] Test floats\n");
		{
			Assert::AreEqual(3.0f, FPL_MIN(3.0f, 7.0f), L"", LINE_INFO());
			Assert::AreEqual(3.0f, FPL_MIN(7.0f, 3.0f), L"", LINE_INFO());
			Assert::AreEqual(-7.0f, FPL_MIN(-7.0f, -3.0f), L"", LINE_INFO());
			Assert::AreEqual(-7.0f, FPL_MIN(-3.0f, -7.0f), L"", LINE_INFO());
			struct TestStruct {
				float a;
				float b;
			};
			TestStruct instance = { 3.0f, 7.0f };
			TestStruct *instancePtr = &instance;
			Assert::AreEqual(3.0f, FPL_MIN(instancePtr->a, instancePtr->b), L"", LINE_INFO());
		}
		Msg("[FPL_MAX] Test integers\n");
		{
			Assert::AreEqual(7, FPL_MAX(3, 7), L"", LINE_INFO());
			Assert::AreEqual(7, FPL_MAX(7, 3), L"", LINE_INFO());
			Assert::AreEqual(-3, FPL_MAX(-3, -7), L"", LINE_INFO());
			Assert::AreEqual(-3, FPL_MAX(-7, -3), L"", LINE_INFO());
			struct TestStruct {
				int a;
				int b;
			};
			TestStruct instance = { 3, 7 };
			TestStruct *instancePtr = &instance;
			Assert::AreEqual(7, FPL_MAX(instancePtr->a, instancePtr->b), L"", LINE_INFO());
		}
		Msg("[FPL_MAX] Test floats\n");
		{
			Assert::AreEqual(7.0f, FPL_MAX(3.0f, 7.0f), L"", LINE_INFO());
			Assert::AreEqual(7.0f, FPL_MAX(7.0f, 3.0f), L"", LINE_INFO());
			Assert::AreEqual(-3.0f, FPL_MAX(-3.0f, -7.0f), L"", LINE_INFO());
			Assert::AreEqual(-3.0f, FPL_MAX(-7.0f, -3.0f), L"", LINE_INFO());
			struct TestStruct {
				float a;
				float b;
			};
			TestStruct instance = { 3.0f, 7.0f };
			TestStruct *instancePtr = &instance;
			Assert::AreEqual(7.0f, FPL_MAX(instancePtr->a, instancePtr->b), L"", LINE_INFO());
		}

		//
		// FPL_KILOBYTES, FPL_MEGABYTES, ...
		//
		Msg("[FPL_KILOBYTES] Test 8 KB \n");
		Assert::AreEqual((size_t)8192, (size_t)FPL_KILOBYTES(8), L"", LINE_INFO());
		Msg("[FPL_MEGABYTES] Test 8 MB \n");
		Assert::AreEqual((size_t)8388608, (size_t)FPL_MEGABYTES(8), L"", LINE_INFO());
		Msg("[FPL_GIGABYTES] Test 1 GB \n");
		Assert::AreEqual((size_t)1073741824, (size_t)FPL_GIGABYTES(1), L"", LINE_INFO());
		Msg("[FPL_GIGABYTES] Test 4 GB \n");
		Assert::AreEqual((size_t)4294967296, (size_t)FPL_GIGABYTES(4), L"", LINE_INFO());
		Msg("[FPL_TERABYTES] Test 2 TB \n");
		Assert::AreEqual((size_t)2199023255552, (size_t)FPL_TERABYTES(2), L"", LINE_INFO());
	}

	TEST_METHOD(TestAtomics) {
		Msg("Test AtomicExchangeU32 with different values\n");
		{
			constexpr uint32_t expectedBefore = 42;
			constexpr uint32_t expectedAfter = 1337;
			volatile uint32_t t = expectedBefore;
			uint32_t r = AtomicExchangeU32(&t, expectedAfter);
			Assert::AreEqual(expectedBefore, r, L"", LINE_INFO());
			Assert::AreEqual(expectedAfter, (uint32_t)t, L"", LINE_INFO());
		}
		Msg("Test AtomicExchangeU32 with negative value\n");
		{
			constexpr uint32_t expectedBefore = 42;
			constexpr uint32_t exchangeValue = -1;
			constexpr uint32_t expectedAfter = (uint32_t)UINT32_MAX;
			volatile uint32_t t = expectedBefore;
			uint32_t r = AtomicExchangeU32(&t, exchangeValue);
			Assert::AreEqual(expectedBefore, r, L"", LINE_INFO());
			Assert::AreEqual(expectedAfter, (uint32_t)t, L"", LINE_INFO());
		}
		Msg("Test AtomicExchangeU32 with same value\n");
		{
			constexpr uint32_t expectedBefore = 1;
			constexpr uint32_t exchangeValue = expectedBefore;
			constexpr uint32_t expectedAfter = exchangeValue;
			volatile uint32_t t = expectedBefore;
			uint32_t r = AtomicExchangeU32(&t, exchangeValue);
			Assert::AreEqual(expectedBefore, r, L"", LINE_INFO());
			Assert::AreEqual(expectedAfter, (uint32_t)t, L"", LINE_INFO());
		}
		Msg("Test AtomicExchangeS32 with different values\n");
		{
			constexpr int32_t expectedBefore = 42;
			constexpr int32_t exchangeValue = 1337;
			constexpr int32_t expectedAfter = exchangeValue;
			volatile int32_t t = expectedBefore;
			int32_t r = AtomicExchangeS32(&t, exchangeValue);
			Assert::AreEqual(expectedBefore, r, L"", LINE_INFO());
			Assert::AreEqual(expectedAfter, (int32_t)t, L"", LINE_INFO());
		}
		Msg("Test AtomicExchangeS32 with negative value\n");
		{
			constexpr int32_t expectedBefore = 42;
			constexpr uint32_t exchangeValue = -1;
			constexpr int32_t expectedAfter = exchangeValue;
			volatile int32_t t = expectedBefore;
			int32_t r = AtomicExchangeS32(&t, exchangeValue);
			Assert::AreEqual(expectedBefore, r, L"", LINE_INFO());
			Assert::AreEqual(expectedAfter, (int32_t)t, L"", LINE_INFO());
		}
		Msg("Test AtomicExchangeS32 with same value\n");
		{
			constexpr int32_t expectedBefore = 1;
			constexpr int32_t exchangeValue = expectedBefore;
			constexpr int32_t expectedAfter = exchangeValue;
			volatile int32_t t = expectedBefore;
			int32_t r = AtomicExchangeS32(&t, exchangeValue);
			Assert::AreEqual(expectedBefore, r, L"", LINE_INFO());
			Assert::AreEqual(expectedAfter, (int32_t)t, L"", LINE_INFO());
		}

		Msg("Test AtomicExchangeU64 with different values\n");
		{
			constexpr uint64_t expectedBefore = 42;
			constexpr uint64_t expectedAfter = 1337;
			volatile uint64_t t = expectedBefore;
			uint64_t r = AtomicExchangeU64(&t, expectedAfter);
			Assert::AreEqual(expectedBefore, r, L"", LINE_INFO());
			Assert::AreEqual(expectedAfter, (uint64_t)t, L"", LINE_INFO());
		}
		Msg("Test AtomicExchangeU64 with negative value\n");
		{
			constexpr uint64_t expectedBefore = 42;
			constexpr uint64_t exchangeValue = -1;
			constexpr uint64_t expectedAfter = (uint64_t)UINT64_MAX;
			volatile uint64_t t = expectedBefore;
			uint64_t r = AtomicExchangeU64(&t, exchangeValue);
			Assert::AreEqual(expectedBefore, r, L"", LINE_INFO());
			Assert::AreEqual(expectedAfter, (uint64_t)t, L"", LINE_INFO());
		}
		Msg("Test AtomicExchangeU64 with same value\n");
		{
			constexpr uint64_t expectedBefore = 1;
			constexpr uint64_t exchangeValue = expectedBefore;
			constexpr uint64_t expectedAfter = exchangeValue;
			volatile uint64_t t = expectedBefore;
			uint64_t r = AtomicExchangeU64(&t, exchangeValue);
			Assert::AreEqual(expectedBefore, r, L"", LINE_INFO());
			Assert::AreEqual(expectedAfter, (uint64_t)t, L"", LINE_INFO());
		}
		Msg("Test AtomicExchangeS64 with different values\n");
		{
			constexpr int64_t expectedBefore = 42;
			constexpr int64_t exchangeValue = 1337;
			constexpr int64_t expectedAfter = exchangeValue;
			volatile int64_t t = expectedBefore;
			int64_t r = AtomicExchangeS64(&t, exchangeValue);
			Assert::AreEqual(expectedBefore, r, L"", LINE_INFO());
			Assert::AreEqual(expectedAfter, (int64_t)t, L"", LINE_INFO());
		}
		Msg("Test AtomicExchangeS64 with negative value\n");
		{
			constexpr int64_t expectedBefore = 42;
			constexpr uint64_t exchangeValue = -1;
			constexpr int64_t expectedAfter = exchangeValue;
			volatile int64_t t = expectedBefore;
			int64_t r = AtomicExchangeS64(&t, exchangeValue);
			Assert::AreEqual(expectedBefore, r, L"", LINE_INFO());
			Assert::AreEqual(expectedAfter, (int64_t)t, L"", LINE_INFO());
		}
		Msg("Test AtomicExchangeS64 with same value\n");
		{
			constexpr int64_t expectedBefore = 1;
			constexpr int64_t exchangeValue = expectedBefore;
			constexpr int64_t expectedAfter = exchangeValue;
			volatile int64_t t = expectedBefore;
			int64_t r = AtomicExchangeS64(&t, exchangeValue);
			Assert::AreEqual(expectedBefore, r, L"", LINE_INFO());
			Assert::AreEqual(expectedAfter, (int64_t)t, L"", LINE_INFO());
		}

	}

	};
}