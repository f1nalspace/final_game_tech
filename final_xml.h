/***
final_xml.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

A open source single header file XML parser C99 library.

This library is designed to parse a xml-byte stream.
It has bare minimum error handling and a limited set of features.

The main usage is to read xml files, such as .TMX
or other asset xml based file formats.

It uses a block allocator memory scheme based on malloc.
Use FXML_MALLOC/FXML_FREE to provide your own memory allocation function.

The only dependencies are a C99 complaint compiler.

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Drop this file into your main C/C++ project and include it in one place you want.
- Define FXML_IMPLEMENTATION before including this header file in your main translation unit.

-------------------------------------------------------------------------------
	Usage
-------------------------------------------------------------------------------

#define FXML_IMPLEMENTATION
#include <final_xml.h>

char *xmlStream = ...
size_t xmlStreamLen = ...

fxmlContext ctx = FXML_ZERO_INIT;
if(fxmlInitFromMemory(xml1, strlen(xml1), &ctx)) {
	fxmlTag root = FXML_ZERO_INIT;
	if(fxmlParse(&ctx, &root)) {
		// Parsed result is stored in the root tag, including all childrens and attributes
	}
	fxmlFree(&ctx);
}

-------------------------------------------------------------------------------
	License
-------------------------------------------------------------------------------

Final XML is released under the following license:

MIT License

Copyright (c) 2017-2025 Torsten Spaete

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
***/

/*!
	\file final_xml.h
	\version v0.3.1 alpha
	\author Torsten Spaete
	\brief Final XML (FXML) - A open source C99 single file header xml parser library.
*/

/*!
	\page page_changelog Changelog
	\tableofcontents

	## v0.3.1 alpha:
	- Fixed memcpy_s compile error on linux by introducing FXML_MEMCPY, that can be overwritten if needed

	## v0.3.0 alpha:
	- Added typedef fxmlErrorType to fxmlContext
	- Fixed missing error validation
	- Fixed FXML_ASSERT was removing loading code in release builds
	- Fixed missing const for fxmlFindTagByName
	- Fixed missing const for fxmlFindAttributeByName
	- Fixed missing const for fxmlGetAttributeValue
	- Fixed missing const for fxmlGetTagValue

	## v0.2.0 alpha:
	- Fixed critical crash when allocating memory (Wrong capacity)
	- Fixed heap corruption for UTF-8 decoding
	- Improved memory usage (Less and larger allocations)
	- Always use existing blocks for memory allocation first

	## v0.1.0 alpha:
	- Initial version
*/

/*!
	\page page_todo Todo
	\tableofcontents

	- Encoding support
		- ISO8859-1
		- ASCII
		- UTF-8
		- UTF-16 / Unicode

*/

#ifndef FXML_H
#define FXML_H

// Detect compiler
#if !defined(__cplusplus) && ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900)))
#	define FXML_IS_C99
#elif defined(__cplusplus)
#	define FXML_IS_CPP
#else
#	error "This C/C++ compiler is not supported!"
#endif

// Api export
#if defined(FXML_PRIVATE)
#	define fxml_api static
#else
#	define fxml_api extern
#endif

// Malloc functions override
#ifndef FXML_MALLOC
#	include <malloc.h>
#	define FXML_MALLOC(size) malloc(size)
#	define FXML_FREE(ptr) free(ptr)
#endif
#ifndef FXML_MEMSET
#	include <string.h>
#	define FXML_MEMSET(dst, value, size) memset(dst, value, size)
#endif
#ifndef FXML_MEMCPY
#	include <string.h>
#	define FXML_MEMCPY(dst, src, size) memcpy(dst, src, size)
#endif
#ifndef FXML_ASSERT
#	include <assert.h>
#	define FXML_ASSERT(exp) assert(exp)
#endif

// Useful macros
#if defined(FXML_IS_C99)
#	define FXML_ZERO_INIT {0}
#else
#	define FXML_ZERO_INIT {}
#endif
#define FXML_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

// Includes
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define fxml_null NULL

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

	typedef enum fxmlTagType {
		fxmlTagType_None = 0,
		fxmlTagType_Root,
		fxmlTagType_Declaration,
		fxmlTagType_Element,
		fxmlTagType_Attribute,
		fxmlTagType_Comment,
	} fxmlTagType;

	typedef struct fxmlString {
		const char *start;
		size_t len;
	} fxmlString;

	typedef struct fxmlMemory {
		void *base;
		struct fxmlMemory *next;
		size_t used;
		size_t capacity;
	} fxmlMemory;

	typedef struct fxmlTag {
		const char *name;
		const char *value;
		struct fxmlTag *parent;
		struct fxmlTag *nextSibling;
		struct fxmlTag *prevSibling;
		struct fxmlTag *firstAttribute;
		struct fxmlTag *lastAttribute;
		struct fxmlTag *firstChild;
		struct fxmlTag *lastChild;
		fxmlTagType type;
		bool isClosed;
	} fxmlTag;

	typedef enum fxmlErrorType {
		fxmlErrorType_None = 0,

		fxmlErrorType_OutOfMemory,
		fxmlErrorType_StringDecodingFailed,
		fxmlErrorType_UnexpectedChar,
		fxmlErrorType_RootTagMissing,
		fxmlErrorType_TagNotClosed,
		fxmlErrorType_ExpectNamespaceIdent,

		fxmlErrorType_ExpectCommentStart,
		fxmlErrorType_ExpectCommentEnd,
		fxmlErrorType_CommentParseError,

		fxmlErrorType_ExpectDeclarationIdent,
		fxmlErrorType_ExpectDeclarationBegin,
		fxmlErrorType_ExpectDeclarationEnd,
		fxmlErrorType_DeclarationParseError,

		fxmlErrorType_ExpectAttributeAssignment,
		fxmlErrorType_ExpectAttributeQuote,
		fxmlErrorType_AttributesParseError,

		fxmlErrorType_ExpectTagStart,
		fxmlErrorType_ExpectTagEnd,
		fxmlErrorType_ExpectTagIdent,
		fxmlErrorType_TagNameTooLong,
		fxmlErrorType_ClosingTagMismatch,
		fxmlErrorType_InvalidTagChar,
		fxmlErrorType_TagParseError,
	} fxmlErrorType;

	typedef struct fxmlContext {
		const void *data;
		const char *ptr;
		size_t size;
		fxmlMemory *firstMem;
		fxmlMemory *lastMem;
		fxmlTag *root;
		fxmlTag *curParent;
		fxmlErrorType errorType;
		bool isError;
	} fxmlContext;

	fxml_api bool fxmlInitFromMemory(const void *data, const size_t dataSize, fxmlContext *outContext);
	fxml_api bool fxmlParse(fxmlContext *context, fxmlTag *outRoot);
	fxml_api void fxmlFree(fxmlContext *context);
	fxml_api const fxmlTag *fxmlFindTagByName(const fxmlTag *tag, const char *name);
	fxml_api const fxmlTag *fxmlFindAttributeByName(const fxmlTag *tag, const char *name);
	fxml_api const char *fxmlGetAttributeValue(const fxmlTag *tag, const char *attrName);
	fxml_api const char *fxmlGetTagValue(const fxmlTag *tag, const char *tagName);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FXML_H

#if defined(FXML_IMPLEMENTATION) && !defined(FXML_IMPLEMENTED)
#define FXML_IMPLEMENTED

#if _DEBUG && 0
#include <crtdbg.h>
#define FXML__HEAPCHECK() FXML_ASSERT(_CrtCheckMemory())
#else
#define FXML__HEAPCHECK()
#endif

#define FXML__MIN_ALLOC_SIZE 4096
#define FXML__MIN_TAG_ALLOC_COUNT 16
#define FXML__BLOCK_PADDING sizeof(uintptr_t)

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

	static inline bool fxml__IsAlpha(const char c) {
		bool result = (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
		return(result);
	}
	static inline bool fxml__IsNumeric(const char c) {
		bool result = (c >= 48 && c <= 57);
		return(result);
	}
	static inline bool fxml__IsAlphaNumeric(const char c) {
		bool result = fxml__IsAlpha(c) || fxml__IsNumeric(c);
		return(result);
	}
	static inline bool fxml__IsWhitespace(const char c) {
		bool result = c == ' ' || c == '\t' || c == '\n' || c == '\r';
		return(result);
	}

	static bool fxml__IsEqualString(const char *a, const char *b) {
		if((a == fxml_null) && (b == fxml_null)) {
			return true;
		}
		if(a == fxml_null || b == fxml_null) {
			return false;
		}
		while(true) {
			if(!*a && !*b) {
				break;
			}
			if((!*a && *b) || (*a && !*b) || (*a != *b)) {
				return false;
			}
			++a;
			++b;
		}
		return(true);
	}

	static void fxml__ReportError(fxmlContext *context, const fxmlErrorType type) {
		if(!context->isError) {
			context->isError = true;
			context->errorType = type;
		}

		// @TODO(final): Log the error out
	}

	static size_t fxml__ComputeBlockSize(const size_t minSize, const size_t blockSize) {
		size_t result = blockSize;
		while(result < minSize) {
			result *= 2;
		}
		return(result);
	}

	static void fxml__FreeMemory(fxmlContext *context) {
		if(context != fxml_null) {
			fxmlMemory *mem = context->firstMem;
			while(mem != fxml_null) {
				fxmlMemory *next = mem->next;
				void *blockBase = mem;
				FXML_FREE(blockBase);
				mem = next;
			}
		}
	}

	static void *fxml__AllocMemory(fxmlContext *context, const size_t size, const size_t allocCount) {
		// Find block which have the required space first
		fxmlMemory *mem = context->firstMem;
		while(mem != fxml_null) {
			if((mem->used + size) <= mem->capacity) {
				break;
			}
			mem = mem->next;
		}

		if(mem == fxml_null) {
			// Allocate new block
			size_t allocationSize = size * allocCount;
			size_t headerMemorySize = sizeof(fxmlMemory) + FXML__BLOCK_PADDING;
			size_t blockSize = fxml__ComputeBlockSize(headerMemorySize + allocationSize, FXML__MIN_ALLOC_SIZE);
			void *blockBase = FXML_MALLOC(blockSize);
			if(blockBase == fxml_null) {
				fxml__ReportError(context, fxmlErrorType_OutOfMemory);
				return(fxml_null);
			}

			FXML_MEMSET(blockBase, 0, blockSize);

			fxmlMemory *newBlock = (fxmlMemory *)blockBase;
			newBlock->capacity = blockSize - headerMemorySize;
			newBlock->used = 0;
			newBlock->base = (uint8_t *)blockBase + headerMemorySize;
			newBlock->next = fxml_null;

			FXML_ASSERT(newBlock->capacity >= allocationSize);

			if(context->lastMem == fxml_null) {
				context->firstMem = context->lastMem = newBlock;
			} else {
				fxmlMemory *lastMem = (fxmlMemory *)context->lastMem;
				lastMem->next = newBlock;
				context->lastMem = newBlock;
			}

			mem = newBlock;
		}


		void *result = (uint8_t *)mem->base + mem->used;
		mem->used += size;

		return(result);
	}

	static fxmlTag *fxml__AllocTag(fxmlContext *context) {
		fxmlTag *mem = (fxmlTag *)fxml__AllocMemory(context, sizeof(fxmlTag), FXML__MIN_TAG_ALLOC_COUNT);
		if(mem == fxml_null) {
			fxml__ReportError(context, fxmlErrorType_OutOfMemory);
			return(fxml_null);
		}
		return(mem);
	}

	static const char *fxml__AllocString(fxmlContext *context, const fxmlString *str) {
		size_t requiredLen = str->len + 1;
		size_t requiredSize = sizeof(char) * requiredLen;
		char *mem = (char *)fxml__AllocMemory(context, requiredSize, 1);
		if(mem == fxml_null) {
			fxml__ReportError(context, fxmlErrorType_OutOfMemory);
			return(fxml_null);
		}
		size_t len = str->len;
		FXML_MEMCPY(mem, str->start, len);
		return(mem);
	}

	static const char *fxml__AllocStringDecode(fxmlContext *context, const fxmlString *str) {
		size_t requiredLen = (str->len * 1) + 1;
		size_t requiredSize = sizeof(char) * requiredLen;
		char *mem = (char *)fxml__AllocMemory(context, requiredSize, 1);
		if(mem == fxml_null) {
			fxml__ReportError(context, fxmlErrorType_OutOfMemory);
			return(fxml_null);
		}

		const char *src = str->start;
		const char *srcEnd = str->start + str->len;
		const char *srcPartStart = str->start;
		char *dst = mem;
		while(src < srcEnd) {
			if(*src == '&') {
				++src;
				if(*src == '#') {
					++src;
					uint64_t escapeCode = 0;
					if(!fxml__IsNumeric(*src)) {
						context->isError = true;
						goto done;
					}
					while(fxml__IsNumeric(*src)) {
						uint32_t v = *src - '0';
						escapeCode = escapeCode * 10 + v;
						++src;
					}
					if(escapeCode > 0 && escapeCode < 256) {
						*dst++ = (char)escapeCode;
					}
				} else if(fxml__IsAlpha(*src)) {
					char symbolName[16 + 1];
					const char *symbolStart = src;
					size_t symbolLen = 0;
					while(fxml__IsAlpha(*src)) {
						size_t symbolIndex = src - symbolStart;
						if(symbolIndex < FXML_ARRAYCOUNT(symbolName)) {
							symbolName[symbolIndex] = *src;
							++symbolLen;
						}
						++src;
					}
					symbolName[symbolLen] = 0;
					if(fxml__IsEqualString(symbolName, "quot")) {
						*dst++ = '\"';
					} else if(fxml__IsEqualString(symbolName, "apos")) {
						*dst++ = '\'';
					} else if(fxml__IsEqualString(symbolName, "amp")) {
						*dst++ = '&';
					} else if(fxml__IsEqualString(symbolName, "lt")) {
						*dst++ = '<';
					} else if(fxml__IsEqualString(symbolName, "gt")) {
						*dst++ = '>';
					}
				}
				if(*src != ';') {
					fxml__ReportError(context, fxmlErrorType_StringDecodingFailed);
					context->isError = true;
					goto done;
				}
				++src;
				continue;
			} else {
				*dst++ = *src;
			}
			++src;
		}

	done:
		*dst = 0;
		return(mem);
	}

	fxml_api bool fxmlInitFromMemory(const void *data, const size_t dataSize, fxmlContext *outContext) {
		if(data == fxml_null || dataSize == 0) {
			return false;
		}
		if(outContext == fxml_null) {
			return false;
		}

		FXML_MEMSET(outContext, 0, sizeof(*outContext));
		outContext->data = data;
		outContext->ptr = (const char *)data;
		outContext->size = dataSize;

		return(true);
	}

	static bool fxml__ParseIdent(fxmlContext *context, fxmlString *outIdent) {
		if(!fxml__IsAlpha(*context->ptr)) {
			return(false);
		}
		const char *start = context->ptr;
		++context->ptr;
		while(fxml__IsAlphaNumeric(*context->ptr) || *context->ptr == '_' || *context->ptr == '-') {
			++context->ptr;
		}
		if(outIdent != fxml_null) {
			outIdent->start = start;
			outIdent->len = context->ptr - start;
		}
		return(true);
	}

	static bool fxml__ParseAttribute(fxmlContext *context, fxmlString *outName, fxmlString *outValue) {
		if(!fxml__IsAlpha(*context->ptr)) {
			// NOTE(final): No attribute is not an error
			return(false);
		}
		fxml__ParseIdent(context, outName);
		if(context->ptr[0] == ':') {
			++context->ptr;
			if(!fxml__IsAlpha(context->ptr[0]) || !fxml__ParseIdent(context, fxml_null)) {
				fxml__ReportError(context, fxmlErrorType_ExpectNamespaceIdent);
				return(false);
			}
			outName->len = context->ptr - outName->start;
		}

		if(*context->ptr != '=') {
			fxml__ReportError(context, fxmlErrorType_ExpectAttributeAssignment);
			return false;
		}
		++context->ptr;

		if(*context->ptr != '\"') {
			fxml__ReportError(context, fxmlErrorType_ExpectAttributeQuote);
			return false;
		}
		++context->ptr;

		outValue->start = context->ptr;
		while(*context->ptr && (*context->ptr != '\"')) {
			++context->ptr;
		}
		outValue->len = context->ptr - outValue->start;

		if(*context->ptr != '\"') {
			fxml__ReportError(context, fxmlErrorType_ExpectAttributeQuote);
			return false;
		}
		++context->ptr;
		return(true);
	}

	static void fxml__SkipWhitespaces(fxmlContext *context) {
		while(!context->isError && fxml__IsWhitespace(*context->ptr)) {
			++context->ptr;
		}
	}

	static void fxml__AddAttribute(fxmlTag *parent, fxmlTag *attr) {
		if(parent->lastAttribute == fxml_null) {
			parent->firstAttribute = parent->lastAttribute = attr;
		} else {
			attr->prevSibling = parent->lastAttribute;
			parent->lastAttribute->nextSibling = attr;
			parent->lastAttribute = attr;
		}

	}

	static bool fxml__ParseAttributes(fxmlContext *context, fxmlTag *parent) {
		while(!context->isError && *context->ptr) {
			fxml__SkipWhitespaces(context);
			fxmlString attrName = FXML_ZERO_INIT;
			fxmlString attrValue = FXML_ZERO_INIT;
			if(!fxml__ParseAttribute(context, &attrName, &attrValue)) {
				break;
			} else {
				fxmlTag *attr = fxml__AllocTag(context);
				if(attr == fxml_null) {
					fxml__ReportError(context, fxmlErrorType_OutOfMemory);
					return(false);
				}
				attr->type = fxmlTagType_Attribute;
				attr->name = fxml__AllocString(context, &attrName);
				attr->value = fxml__AllocStringDecode(context, &attrValue);
				fxml__AddAttribute(parent, attr);
			}
		}
		fxml__SkipWhitespaces(context);
		return(true);
	}

	static void fxml__AddChild(fxmlTag *parent, fxmlTag *child) {
		if(parent->lastChild == fxml_null) {
			parent->firstChild = parent->lastChild = child;
		} else {
			child->prevSibling = parent->lastChild;
			parent->lastChild->nextSibling = child;
			parent->lastChild = child;
		}
	}

	static bool fxml__ParseComment(fxmlContext *context) {
		if(context->ptr[0] != '<' || context->ptr[1] != '!') {
			fxml__ReportError(context, fxmlErrorType_ExpectCommentStart);
			return(false);
		}
		context->ptr += 2;

		if(context->ptr[0] != '-' || context->ptr[1] != '-') {
			fxml__ReportError(context, fxmlErrorType_ExpectCommentStart);
			return(false);
		}
		context->ptr += 2;

		fxmlString comment = FXML_ZERO_INIT;
		comment.start = context->ptr;
		while(!context->isError && *context->ptr) {
			if(context->ptr[0] == '-') {
				if(context->ptr[1] == '-') {
					if(context->ptr[2] != '>') {
						fxml__ReportError(context, fxmlErrorType_ExpectCommentEnd);
						return(false);
					} else {
						break;
					}
				}
			}
			++context->ptr;
		}
		comment.len = context->ptr - comment.start;

		fxmlTag *commentTag = fxml__AllocTag(context);
		if(commentTag == fxml_null) {
			fxml__ReportError(context, fxmlErrorType_OutOfMemory);
			return(false);
		}

		commentTag->value = fxml__AllocStringDecode(context, &comment);
		commentTag->type = fxmlTagType_Comment;

		FXML_ASSERT(context->curParent != fxml_null);

		fxml__AddChild(context->curParent, commentTag);

		if(context->ptr[0] != '-' || context->ptr[1] != '-' || context->ptr[2] != '>') {
			fxml__ReportError(context, fxmlErrorType_ExpectCommentEnd);
			return(false);
		}
		context->ptr += 3;
		return(true);
	}

	static fxmlTag *fxml__ParseDeclaration(fxmlContext *context) {
		if(context->ptr[0] != '<' || context->ptr[1] != '?') {
			fxml__ReportError(context, fxmlErrorType_ExpectDeclarationBegin);
			return(fxml_null);
		}

		context->ptr += 2;

		fxmlString declName = FXML_ZERO_INIT;
		if(!fxml__IsAlpha(*context->ptr) || !fxml__ParseIdent(context, &declName)) {
			fxml__ReportError(context, fxmlErrorType_ExpectDeclarationIdent);
			return(fxml_null);
		}

		fxmlTag *declTag = fxml__AllocTag(context);
		if(declTag == fxml_null) {
			fxml__ReportError(context, fxmlErrorType_OutOfMemory);
			return(fxml_null);
		}

		declTag->name = fxml__AllocString(context, &declName);
		declTag->type = fxmlTagType_Declaration;
		if(!fxml__ParseAttributes(context, declTag)) {
			fxml__ReportError(context, fxmlErrorType_AttributesParseError);
			return(fxml_null);
		}

		fxml__AddChild(context->root, declTag);

		if(context->ptr[0] != '?' || context->ptr[1] != '>') {
			fxml__ReportError(context, fxmlErrorType_ExpectDeclarationEnd);
			return(fxml_null);
		}
		context->ptr += 2;

		return(declTag);
	}

	typedef enum fxml__ParseTagMode {
		fxml__ParseTagMode_None = 0,
		fxml__ParseTagMode_Open,
		fxml__ParseTagMode_Close,
		fxml__ParseTagMode_OpenAndClose,
	} fxml__ParseTagMode;

	typedef struct fxml__ParseTagResult {
		fxml__ParseTagMode mode;
		fxmlTag *tag;
		char tagName[256];
	} fxml__ParseTagResult;

	static bool fxml__ParseTag(fxmlContext *context, fxml__ParseTagResult *outResult) {
		if(context->ptr[0] != '<') {
			fxml__ReportError(context, fxmlErrorType_ExpectTagStart);
			return(false);
		}

		outResult->mode = fxml__ParseTagMode_Open;
		outResult->tag = fxml_null;
		outResult->tagName[0] = 0;

		context->ptr++;
		if(context->ptr[0] == '/') {
			outResult->mode = fxml__ParseTagMode_Close;
			context->ptr++;
		}

		fxmlString identStr = FXML_ZERO_INIT;
		if(!fxml__IsAlpha(*context->ptr) || !fxml__ParseIdent(context, &identStr)) {
			fxml__ReportError(context, fxmlErrorType_ExpectTagIdent);
			return(false);
		}

		size_t requiredTagNameLength = identStr.len + 1;
		if(requiredTagNameLength > FXML_ARRAYCOUNT(outResult->tagName)) {
			fxml__ReportError(context, fxmlErrorType_TagNameTooLong);
			return(false);
		}

		for(size_t i = 0; i < identStr.len; ++i) {
			outResult->tagName[i] = *(identStr.start + i);
		}
		outResult->tagName[identStr.len] = 0;

		if(context->ptr[0] == ':') {
			// First ident was namespace, parse real ident
			context->ptr++;
			if(!fxml__IsAlpha(context->ptr[0]) || !fxml__ParseIdent(context, fxml_null)) {
				fxml__ReportError(context, fxmlErrorType_ExpectNamespaceIdent);
				return(false);
			}
			identStr.len = context->ptr - identStr.start;
		}

		if(outResult->mode != fxml__ParseTagMode_Close) {
			fxmlTag *tag = fxml__AllocTag(context);
			if(tag == fxml_null) {
				fxml__ReportError(context, fxmlErrorType_OutOfMemory);
				return(false);
			}

			tag->type = fxmlTagType_Element;
			tag->name = fxml__AllocString(context, &identStr);
			tag->parent = context->curParent;
			tag->isClosed = false;
			outResult->tag = tag;
			fxml__AddChild(context->curParent, tag);

			if(!fxml__ParseAttributes(context, tag)) {
				fxml__ReportError(context, fxmlErrorType_AttributesParseError);
				return(false);
			}

			if(context->ptr[0] == '/') {
				outResult->mode = fxml__ParseTagMode_OpenAndClose;
				tag->isClosed = true;
				++context->ptr;
			}
		} else {
			fxml__SkipWhitespaces(context);
		}

		if(context->ptr[0] != '>') {
			fxml__ReportError(context, fxmlErrorType_ExpectTagEnd);
			return(false);
		}
		context->ptr++;
		return(true);
	}

	static void fxml__ParseInnerText(fxmlContext *context, fxmlTag *tag) {
		const char *start = context->ptr;
		while(!context->isError && context->ptr[0] && context->ptr[0] != '<') {
			++context->ptr;
		}
		fxmlString value = FXML_ZERO_INIT;
		value.len = context->ptr - start;
		value.start = start;
		tag->value = fxml__AllocStringDecode(context, &value);
	}

	fxml_api bool fxmlParse(fxmlContext *context, fxmlTag *outRoot) {
		// Read unicode BOM
		bool isUTF8 = false;
		if(context->size >= 4) {
			uint8_t *p = (uint8_t *)context->ptr;
			if(p[0] == 0xFF && p[1] == 0xFE) {
				// Error: UTF-16LE not supported
				context->isError = true;
				return false;
			} else if(p[0] == 0xFE && p[1] == 0xFF) {
				// Error: UTF-16BE not supported
				context->isError = true;
				return false;
			}
			if(p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) {
				// UTF-8 BOM detected
				context->ptr += 3;
				isUTF8 = true;
			}
		}

		outRoot->type = fxmlTagType_Root;
		context->root = outRoot;
		context->curParent = outRoot;
		while(!context->isError && *context->ptr) {
			char c = context->ptr[0];
			bool readAhead = true;
			switch(c) {
				case '<':
				{
					if(context->ptr[1] == '?') {
						fxmlTag *declTag = fxml__ParseDeclaration(context);
						if(context->isError) {
							fxml__ReportError(context, fxmlErrorType_DeclarationParseError);
							break;
						}
						const char *encoding = fxmlGetAttributeValue(declTag, "encoding");
						if(fxml__IsEqualString(encoding, "UTF-8") || fxml__IsEqualString(encoding, "utf-8")) {
							isUTF8 = true;
						}
						readAhead = false;
					} else if(context->ptr[1] == '/' || fxml__IsAlpha(context->ptr[1])) {
						fxml__ParseTagResult tagRes = FXML_ZERO_INIT;
						if(!fxml__ParseTag(context, &tagRes)) {
							fxml__ReportError(context, fxmlErrorType_TagParseError);
							break;
						}
						if(tagRes.mode == fxml__ParseTagMode_Open) {
							fxml__ParseInnerText(context, tagRes.tag);
							context->curParent = tagRes.tag;
						} else if(tagRes.mode == fxml__ParseTagMode_Close) {
							if(!fxml__IsEqualString(context->curParent->name, tagRes.tagName)) {
								fxml__ReportError(context, fxmlErrorType_ClosingTagMismatch);
								break;
							}
							context->curParent->isClosed = true;
							if(context->curParent->parent != fxml_null) {
								context->curParent = context->curParent->parent;
							} else {
								context->curParent = context->root;
							}
						}
						readAhead = false;
					} else if(context->ptr[1] == '!') {
						if(!fxml__ParseComment(context)) {
							fxml__ReportError(context, fxmlErrorType_CommentParseError);
							break;
						}
						readAhead = false;
					} else {
						fxml__ReportError(context, fxmlErrorType_UnexpectedChar);
						return false;
					}
				} break;

				default:
				{

				} break;
			}
			if(readAhead) {
				++context->ptr;
			}
		}

		if(context->curParent == context->root) {
			context->curParent->isClosed = true;
		}

		fxmlTag *testChild = context->root->firstChild;
		int elementCount = 0;
		while(testChild != fxml_null) {
			if(testChild->type == fxmlTagType_Element) {
				++elementCount;
			}
			testChild = testChild->nextSibling;
		}
		if(elementCount != 1) {
			fxml__ReportError(context, fxmlErrorType_RootTagMissing); // No root tag found
			return(false);
		}
		if(!context->curParent->isClosed) {

			fxml__ReportError(context, fxmlErrorType_TagNotClosed); // Last tag not closed
			return(false);
		}

		return(!context->isError);
	}

	fxml_api void fxmlFree(fxmlContext *context) {
		if(context != fxml_null) {
			fxml__FreeMemory(context);
		}
	}

	fxml_api const fxmlTag *fxmlFindTagByName(const fxmlTag *tag, const char *name) {
		const fxmlTag *result = fxml_null;
		if(tag != fxml_null) {
			const fxmlTag *searchTag = tag->firstChild;
			while(searchTag != fxml_null) {
				if(searchTag->type == fxmlTagType_Element && fxml__IsEqualString(searchTag->name, name)) {
					result = searchTag;
					break;
				}
				searchTag = searchTag->nextSibling;
			}
		}
		return(result);
	}

	fxml_api const fxmlTag *fxmlFindAttributeByName(const fxmlTag *tag, const char *name) {
		const fxmlTag *result = fxml_null;
		if(tag != fxml_null) {
			const fxmlTag *searchAttr = tag->firstAttribute;
			while(searchAttr != fxml_null) {
				if(searchAttr->type == fxmlTagType_Attribute && fxml__IsEqualString(searchAttr->name, name)) {
					result = searchAttr;
					break;
				}
				searchAttr = searchAttr->nextSibling;
			}
		}
		return(result);
	}

	fxml_api const char *fxmlGetAttributeValue(const fxmlTag *tag, const char *attrName) {
		const fxmlTag *foundAttr = fxmlFindAttributeByName(tag, attrName);
		if(foundAttr != fxml_null) {
			return foundAttr->value;
		}
		return fxml_null;
	}

	fxml_api const char *fxmlGetTagValue(const fxmlTag *tag, const char *tagName) {
		const fxmlTag *foundTag = fxmlFindTagByName(tag, tagName);
		if(foundTag != fxml_null) {
			return foundTag->value;
		}
		return fxml_null;
	}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FXML_IMPLEMENTATION