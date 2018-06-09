#ifndef FXML_H
#define FXML_H

#if !defined(__cplusplus) && ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900)))
	//! Detected C99 compiler
#	define FXML_IS_C99
#elif defined(__cplusplus)
	//! Detected C++ compiler
#	define FXML_IS_CPP
#else
#	error "This C/C++ compiler is not supported!"
#endif

#if defined(FXML_PRIVATE)
#	define fxml_api static
#else
#	define fxml_api extern
#endif

#ifndef FXML_MALLOC
#	include <malloc.h>
#	define FXML_MALLOC malloc
#endif
#ifndef FXML_FREE
#	include <malloc.h>
#	define FXML_FREE free
#endif
#ifndef FXML_MEMSET
#	include <string.h>
#	define FXML_MEMSET memset
#endif

#if defined(FXML_IS_C99)
	//! Initialize a struct to zero (C99)
#	define FXML_ZERO_INIT {0}
#else
	//! Initialize a struct to zero (C++)
#	define FXML_ZERO_INIT {}
#endif


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

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
	} fxmlTagType;

	typedef struct fxmlTag {
		char *name;
		char *value;
		struct fxmlTag *parent;
		struct fxmlTag *next;
		struct fxmlTag *firstAttribute;
		struct fxmlTag *lastAttribute;
		struct fxmlTag *firstChild;
		struct fxmlTag *lastChild;
		fxmlTagType type;
	} fxmlTag;

	typedef struct fxmlContext {
		const void *data;
		const char *ptr;
		size_t size;
		void *firstMem;
		void *lastMem;
		fxmlTag *root;
		fxmlTag *curParent;
	} fxmlContext;

	fxml_api bool fxmlInitFromMemory(const void *data, const size_t dataSize, fxmlContext *outContext);
	fxml_api bool fxmlParse(fxmlContext *context, fxmlTag *outRoot);
	fxml_api void fxmlFree(fxmlContext *context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FXML_H

#if defined(FXML_IMPLEMENTATION) && !defined(FXML_IMPLEMENTED)
#define FXML_IMPLEMENTED

#define FXML__MIN_ALLOC_SIZE 64

typedef struct fxml__Memory {
	void *base;
	struct fxml__Memory *next;
	size_t used;
	size_t capacity;
} fxml__Memory;

typedef struct fxml__string {
	const char *start;
	size_t len;
} fxml__string;

static size_t fxml__ComputeBlockSize(const size_t size) {
	size_t result = FXML__MIN_ALLOC_SIZE;
	while (result < size) {
		result *= 16;
	}
	return(result);
}

static void *fxml__AllocMemory(fxmlContext *context, const size_t size) {
	if (context->lastMem != fxml_null) {
		fxml__Memory *mem = (fxml__Memory *)context->lastMem;
		if ((mem->used + size) <= mem->capacity) {
			void *result = (uint8_t *)mem->base + mem->used;
			mem->used += size;
			return(result);
		}
	}

	size_t blockSize = fxml__ComputeBlockSize(size);
	size_t totalSize = sizeof(fxml__Memory) + sizeof(intptr_t) + blockSize;
	void *blockBase = FXML_MALLOC(totalSize);
	FXML_MEMSET(blockBase, 0, totalSize);

	fxml__Memory *newBlock = (fxml__Memory *)blockBase;
	newBlock->capacity = blockSize;
	newBlock->used = 0;
	newBlock->base = (uint8_t *)blockBase + sizeof(fxml__Memory) + sizeof(intptr_t);
	newBlock->next = fxml_null;

	if (context->lastMem == fxml_null) {
		context->firstMem = context->lastMem = newBlock;
	} else {
		fxml__Memory *lastMem = (fxml__Memory *)context->lastMem;
		lastMem->next = newBlock;
		context->lastMem = newBlock;
	}

	void *result = (uint8_t *)newBlock->base + newBlock->used;
	newBlock->used += size;
	return(result);
}

static fxmlTag *fxml__AllocTag(fxmlContext *context) {
	fxmlTag *mem = (fxmlTag *)fxml__AllocMemory(context, sizeof(fxmlTag));
	return(mem);
}

static char *fxml__AllocString(fxmlContext *context, const fxml__string *str) {
	char *mem = (char *)fxml__AllocMemory(context, sizeof(char) * (str->len + 1));
	size_t len = str->len;
	char *p = mem;
	while (len > 0) {
		*p = *(str->start + (p - mem));
		++p;
		len--;
	}
	return(mem);
}

fxml_api bool fxmlInitFromMemory(const void *data, const size_t dataSize, fxmlContext *outContext) {
	if (data == fxml_null || dataSize == 0) {
		return false;
	}
	if (outContext == fxml_null) {
		return false;
	}

	FXML_MEMSET(outContext, 0, sizeof(*outContext));
	outContext->data = data;
	outContext->ptr = (const char *)data;
	outContext->size = dataSize;

	return(true);
}

inline bool fxml__IsAlpha(const char c) {
	bool result = (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
	return(result);
}
inline bool fxml__IsNumeric(const char c) {
	bool result = (c >= 48 && c <= 57);
	return(result);
}
inline bool fxml__IsAlphaNumeric(const char c) {
	bool result = fxml__IsAlpha(c) || fxml__IsNumeric(c);
	return(result);
}
inline bool fxml__IsWhitespace(const char c) {
	bool result = c == ' ' || c == '\t' || c == '\n' || c == '\r';
	return(result);
}

static void fxml__ParseIdent(fxmlContext *context, fxml__string *outIdent) {
	assert(fxml__IsAlpha(*context->ptr));
	const char *start = context->ptr;
	++context->ptr;
	while (fxml__IsAlphaNumeric(*context->ptr) || *context->ptr == '_' || *context->ptr == '-') {
		++context->ptr;
	}
	if (outIdent != fxml_null) {
		outIdent->start = start;
		outIdent->len = context->ptr - start;
	}
}

static bool fxml__ParseAttribute(fxmlContext *context, fxml__string *outName, fxml__string *outValue) {
	bool result = false;
	if (fxml__IsAlpha(*context->ptr)) {
		fxml__ParseIdent(context, outName);
		if (context->ptr[0] == ':') {
			++context->ptr;
			fxml__ParseIdent(context, fxml_null);
			outName->len = context->ptr - outName->start;
		}

		assert(*context->ptr == '=');
		++context->ptr;

		assert(*context->ptr == '\"');
		++context->ptr;

		outValue->start = context->ptr;
		while (*context->ptr && (*context->ptr != '\"')) {
			++context->ptr;
		}
		outValue->len = context->ptr - outValue->start;

		assert(*context->ptr == '\"');
		++context->ptr;
		result = true;
	}
	return(result);
}

static void fxml__SkipWhitespaces(fxmlContext *context) {
	while (fxml__IsWhitespace(*context->ptr)) {
		++context->ptr;
	}
}

static void fxml__AddAttribute(fxmlTag *parent, fxmlTag *attr) {
	if (parent->lastAttribute == fxml_null) {
		parent->firstAttribute = parent->lastAttribute = attr;
	} else {
		parent->lastAttribute->next = attr;
		parent->lastAttribute = attr;
	}

}

static void fxml__ParseAttributes(fxmlContext *context, fxmlTag *parent) {
	while (*context->ptr) {
		fxml__SkipWhitespaces(context);
		fxml__string attrName = FXML_ZERO_INIT;
		fxml__string attrValue = FXML_ZERO_INIT;
		if (!fxml__ParseAttribute(context, &attrName, &attrValue)) {
			break;
		} else {
			fxmlTag *attr = fxml__AllocTag(context);
			attr->type = fxmlTagType_Attribute;
			attr->name = fxml__AllocString(context, &attrName);
			attr->value = fxml__AllocString(context, &attrValue);
			fxml__AddAttribute(parent, attr);
		}
	}
	fxml__SkipWhitespaces(context);
}

static void fxml__AddChild(fxmlTag *parent, fxmlTag *child) {
	if (parent->lastChild == fxml_null) {
		parent->firstChild = parent->lastChild = child;
	} else {
		parent->lastChild->next = child;
		parent->lastChild = child;
	}
}

static void fxml__ParseDeclaration(fxmlContext *context) {
	assert(context->ptr[0] == '<');
	assert(context->ptr[1] == '?');
	context->ptr += 2;

	assert(fxml__IsAlpha(*context->ptr));
	fxml__string declName = FXML_ZERO_INIT;
	fxml__ParseIdent(context, &declName);

	fxmlTag *declTag = fxml__AllocTag(context);
	declTag->name = fxml__AllocString(context, &declName);
	declTag->type = fxmlTagType_Declaration;

	fxml__ParseAttributes(context, declTag);
	assert(context->ptr[0] == '?');
	assert(context->ptr[1] == '>');
	context->ptr += 2;

	fxml__AddChild(context->root, declTag);
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
} fxml__ParseTagResult;

static void fxml__ParseTag(fxmlContext *context, fxml__ParseTagResult *outResult) {
	outResult->mode = fxml__ParseTagMode_Open;
	outResult->tag = fgl_null;

	assert(context->ptr[0] == '<');
	context->ptr++;
	if (context->ptr[0] == '/') {
		outResult->mode = fxml__ParseTagMode_Close;
		context->ptr++;
	}
	assert(fxml__IsAlpha(*context->ptr));
	fxml__string identStr = FXML_ZERO_INIT;
	fxml__ParseIdent(context, &identStr);

	if (context->ptr[0] == ':') {
		context->ptr++;
		fxml__ParseIdent(context, fxml_null);
		identStr.len = context->ptr - identStr.start;
	}

	if (outResult->mode != fxml__ParseTagMode_Close) {
		fxmlTag *tag = fxml__AllocTag(context);
		tag->type = fxmlTagType_Element;
		tag->name = fxml__AllocString(context, &identStr);
		tag->parent = context->curParent;
		outResult->tag = tag;
		fxml__AddChild(context->curParent, tag);

		fxml__ParseAttributes(context, tag);
		if (context->ptr[0] == '/') {
			outResult->mode = fxml__ParseTagMode_OpenAndClose;
			++context->ptr;
		}
	}

	assert(context->ptr[0] == '>');
	context->ptr++;
}

static void fxml__ParseInnerText(fxmlContext *context, fxmlTag *tag) {
	fxml__SkipWhitespaces(context);
	const char *start = context->ptr;
	while (context->ptr[0] && context->ptr[0] != '<') {
		++context->ptr;
	}
	fxml__string value = FXML_ZERO_INIT;
	value.len = context->ptr - start;
	value.start = start;
	tag->value = fxml__AllocString(context, &value);
}

fxml_api bool fxmlParse(fxmlContext *context, fxmlTag *outRoot) {
	outRoot->type = fxmlTagType_Root;
	context->root = outRoot;
	context->curParent = outRoot;
	while (*context->ptr) {
		char c = context->ptr[0];
		bool readAhead = true;
		switch (c) {
			case '<':
			{
				if (context->ptr[1] == '?') {
					fxml__ParseDeclaration(context);
					readAhead = false;
				} else if (context->ptr[1] == '/' || fxml__IsAlpha(context->ptr[1])) {
					fxml__ParseTagResult tagRes = FXML_ZERO_INIT;
					fxml__ParseTag(context, &tagRes);
					if (tagRes.mode == fxml__ParseTagMode_Open) {
						fxml__ParseInnerText(context, tagRes.tag);
						context->curParent = tagRes.tag;
					} else if (tagRes.mode == fxml__ParseTagMode_Close) {
						if (context->curParent->parent != fxml_null) {
							context->curParent = context->curParent->parent;
						} else {
							context->curParent = context->root;
						}
					}
					readAhead = false;
				} else {
					return false;
				}
			} break;

			default:
			{

			} break;
		}
		if (readAhead) {
			++context->ptr;
		}
	}
	return(true);
}

fxml_api void fxmlFree(fxmlContext *context) {
	if (context != fxml_null) {
		fxml__Memory *mem = (fxml__Memory *)context->firstMem;
		while (mem != fxml_null) {
			fxml__Memory *next = mem->next;
			void *blockBase = (uint8_t *)mem->base - sizeof(uintptr_t) - sizeof(fxml__Memory);
			FXML_FREE(blockBase);
			mem = next;
		}
	}
}

#endif // FXML_IMPLEMENTATION