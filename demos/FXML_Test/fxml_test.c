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

#define FXML_IMPLEMENTATION
#include <final_xml.h>

#include <string.h>
#include <assert.h>

static bool fxmlTestSuccess(const char *xmlStream) {
	bool parseRes = false;
	fxmlContext ctx = FXML_ZERO_INIT;
	if(fxmlInitFromMemory(xmlStream, strlen(xmlStream), &ctx)) {
		fxmlTag root = FXML_ZERO_INIT;
		parseRes = fxmlParse(&ctx, &root);
		fxmlFree(&ctx);
	}
	return(parseRes);
}

static void fxmlTests() {
	assert(!fxmlTestSuccess(""));
	assert(!fxmlTestSuccess("b"));
	assert(!fxmlTestSuccess("<b"));
	assert(!fxmlTestSuccess("<b>"));
	assert(!fxmlTestSuccess("</b>"));
	assert(!fxmlTestSuccess("< b></b>"));
	assert(!fxmlTestSuccess("<b></ b>"));
	assert(!fxmlTestSuccess("< b></ b>"));
	assert(!fxmlTestSuccess("<b>< /b>"));
	assert(!fxmlTestSuccess("<a></a><b></b>"));
	assert(fxmlTestSuccess("<b ></b >"));
	assert(fxmlTestSuccess("<b></b>"));
	assert(fxmlTestSuccess("<b/>"));
	assert(fxmlTestSuccess("<b />"));
}

int main(int argc, char **argv) {
	fxmlTests();

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
		"<body>Norwegian: æøå. French: êèé</body>\n"
		"<addon>&quot;hello&apos; &#169; &lt;-&gt; &amp;world!</addon>\n"
		"</meta>\n"
		"</root>\n"
	};

	fxmlContext ctx = FXML_ZERO_INIT;
	if(fxmlInitFromMemory(xml1, strlen(xml1), &ctx)) {
		fxmlTag root = FXML_ZERO_INIT;
		if(fxmlParse(&ctx, &root)) {
			fxmlTag *childTag = root.firstChild;
			while(childTag != fxml_null) {
				childTag = childTag->nextSibling;
			}
		}
		fxmlFree(&ctx);
	}

	return 0;
}