/*
-------------------------------------------------------------------------------
Name:
	FXML | Test

Description:
	This demo shows how to use the "Final XML" library, a simple XML parser.

Requirements:
	- C99

Author:
	Torsten Spaete

Changelog:
	## 2018-06-29
	- Initial version

License:
	Copyright (c) 2017-2019 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#include <string.h>
#include <stdio.h>

#if 0
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

static void *MyMalloc(const size_t size);
static void MyFree(void *ptr);

#define FXML_IMPLEMENTATION
#define FXML_MALLOC MyMalloc
#define FXML_FREE MyFree
#include <final_xml.h>

#define TEST_ASSERT(exp) if(!(exp)) {*(int *)0 = 0;}

#define FORCE_MEMORY_MALLOC 1
#define ENABLE_MEMORY_PROTECTION 1

#if _WIN32 && !FORCE_MEMORY_MALLOC
#include <Windows.h>
#else
#include <malloc.h>
#endif

static void *MyMalloc(const size_t size) {
#if _WIN32 && !FORCE_MEMORY_MALLOC

#if ENABLE_MEMORY_PROTECTION
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	size_t pageSize = systemInfo.dwPageSize;
	size_t dataSize = fxml__ComputeBlockSize(size, pageSize);
	size_t allocSize = dataSize + pageSize * 2;
	void *base = VirtualAlloc(NULL, allocSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	void *result = (uint8_t *)base + pageSize; // Data starts after the first page
	void *overflow = (uint8_t *)base + pageSize + dataSize;
	void *underflow = (uint8_t *)base;
	DWORD dummy;
	TEST_ASSERT(VirtualProtect(overflow, pageSize, PAGE_READWRITE | PAGE_GUARD, &dummy));
	TEST_ASSERT(VirtualProtect(underflow, pageSize, PAGE_READWRITE | PAGE_GUARD, &dummy));
#else
	void *result = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#endif

#else
	void *result = malloc(size);
	memset(result, 0, size);
#endif
	return(result);
}

static void MyFree(void *ptr) {
#if _WIN32 && !FORCE_MEMORY_MALLOC
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	free(ptr);
#endif
}

static void PrintAttributes(const fxmlTag *tag) {
	const fxmlTag *attr = tag->firstAttribute;
	while (attr != fxml_null) {
		const fxmlTag *next = attr->nextSibling;
		printf("%s=\"%s\"", attr->name, attr->value);
		if (next != fxml_null) {
			printf(" ");
		}
		attr = next;
	}
}

static void PrintChildren(const fxmlTag *tag) {
	const fxmlTag *child = tag->firstChild;
	while (child != fxml_null) {
		printf("<%s", child->name);
		if (child->firstAttribute != fxml_null) {
			printf(" ");
			PrintAttributes(child);
		}
		printf(">");
		PrintChildren(child);
		child = child->nextSibling;
	}
}

static bool fxmlTestSuccess(const char *xmlStream) {
	bool parseRes = false;
	fxmlContext ctx = FXML_ZERO_INIT;
	if (fxmlInitFromMemory(xmlStream, strlen(xmlStream), &ctx)) {
		fxmlTag root = FXML_ZERO_INIT;
		parseRes = fxmlParse(&ctx, &root);
		printf("\n");
		PrintChildren(&root);
		printf("\n");
		fxmlFree(&ctx);
	}
	return(parseRes);
}

static void UnitTests() {
	TEST_ASSERT(!fxmlTestSuccess(""));
	TEST_ASSERT(!fxmlTestSuccess("b"));
	TEST_ASSERT(!fxmlTestSuccess("<b"));
	TEST_ASSERT(!fxmlTestSuccess("<b>"));
	TEST_ASSERT(!fxmlTestSuccess("</b>"));
	TEST_ASSERT(!fxmlTestSuccess("< b></b>"));
	TEST_ASSERT(!fxmlTestSuccess("<b></ b>"));
	TEST_ASSERT(!fxmlTestSuccess("< b></ b>"));
	TEST_ASSERT(!fxmlTestSuccess("<b>< /b>"));
	TEST_ASSERT(!fxmlTestSuccess("<a></a><b></b>"));
	TEST_ASSERT(fxmlTestSuccess("<b ></b >"));
	TEST_ASSERT(fxmlTestSuccess("<b></b>"));
	TEST_ASSERT(fxmlTestSuccess("<b/>"));
	TEST_ASSERT(fxmlTestSuccess("<b />"));
	TEST_ASSERT(fxmlTestSuccess("<r><a/></r>"));
	TEST_ASSERT(fxmlTestSuccess("<r><a/><b/></r>"));
	TEST_ASSERT(fxmlTestSuccess("<x>&quot;</x>"));
	TEST_ASSERT(fxmlTestSuccess("<surname>&#352;umbera</surname>"));
}

static void ManualTest() {
	const char xml1[] = {
		"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
		"<!-- Special char as copyright in comment ® -->\n"
		"<root>\n"
		"<properties>\n"
		"<property name=\"myNumber\" value=\"1337\" />\n"
		"<property name=\"myString\" value=\"Hello World!\" />\n"
		"<property name=\"myFloat\" value=\"1337.456\" />\n"
		"<property />\n"
		"<something></something>\n"
		"</properties>\n"
		"<meta>\n"
		"<description rating=\"5\">The great description here</description>\n"
		"<body>Norwegian: Å/å, Æ/æ, Ø/ø, Ò/ò, French: Französisch (Æ/æ, À/à, Â/â, È/è, É/é, Ê/ê, Ë/ë, Î/î, Ï/ï, Ô/ô, Ù/ù, Û/û, Ç/ç, Ü/ü, ÿ, nicht Œ/œ, Ÿ),</body>\n"
		"<addon>&quot;hello&apos; &#169; &lt;-&gt; &amp;world!</addon>\n"
		"</meta>\n"
		"</root>\n"
	};

	fxmlContext ctx = FXML_ZERO_INIT;
	if (fxmlInitFromMemory(xml1, strlen(xml1), &ctx)) {
		fxmlTag root = FXML_ZERO_INIT;
		if (fxmlParse(&ctx, &root)) {
			fxmlTag *childTag = root.firstChild;
			while (childTag != fxml_null) {
				childTag = childTag->nextSibling;
			}
		}
		fxmlFree(&ctx);
	}
}



static void FileTest(const char *filePath) {
	fxmlContext ctx = FXML_ZERO_INIT;
	FILE *f = fxml_null;
	bool r = fopen_s(&f, filePath, "rb") == 0;
	TEST_ASSERT(r);
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	void *mem = malloc(size);
	fread(mem, size, 1, f);
	fclose(f);
	if (fxmlInitFromMemory(mem, size, &ctx)) {
		fxmlTag root = FXML_ZERO_INIT;
		if (fxmlParse(&ctx, &root)) {
			PrintChildren(&root);
		}
		fxmlFree(&ctx);
	}
	free(mem);
}

int main(int argc, char **argv) {
	UnitTests();
	ManualTest();

#if 0
	if (argc == 2) {
		const char *projectPath = argv[1];
		const char *filename = "level1.tmx";
		char filePath[512] = { 0 };
		strcat_s(filePath, 512, projectPath);
		strcat_s(filePath, 512, filename);
		FileTest(filePath);
	}
#endif

	return 0;
}