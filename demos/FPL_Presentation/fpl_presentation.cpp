/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Presentation

Description:
	A power point presentation like application, which i use to present FPL to users.
	It makes heavy use of OpenGL and defines all slides in a very simple format.

Requirements:
	- C++/11 Compiler
	- Final Platform Layer
	- Final Dynamic OpenGL
	- STB_image
	- STB_truetype

Author:
	Torsten Spaete

Changelog:
	## 2020-15-05
	- Made it much more nicer looking

	## 2020-05-09
	- Initial version

License:
	Copyright (c) 2017-2020 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_PRIVATE
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <stb/stb_truetype.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_STATIC
#include <stb/stb_image.h>

#define FXML_IMPLEMENTATION
#define FXML_PRIVATE
#include <final_xml.h>

#include <final_random.h>

#include <final_math.h> // Vec2f, Vec4f, Mat4f, etc.

// Contains fonts files as byte-array (Arimo, Sulphur-Point, Bitstream Vera Sans)
#include "fonts.h"

// Contains image files as byte-array (FPL-Logos)
#include "images.h"

// Contains the slide text for the FPL presentation
#include "slides.h" // TextDefinition, SlideDefinition

#include "types.h" // HorizontalAlignment, VerticalAlignment

#define DRAW_TEXT_BOUNDS 0
#define DRAW_IMAGE_BOUNDS 0
#define DRAW_SLIDE_CENTER 0
#define DRAW_VIEW_CENTER 0
#define DRAW_ROTATING_CUBE 1
#define USE_LETTERBOX_VIEWPORT 0

template <typename T>
struct GrowablePool {
private:
	struct Entry {
		T value;
		Entry *next;
	};

	struct Bucket {
		Entry *firstFree;
		Bucket *next;
	};

	Entry *firstUsed;
	Entry *lastUsed;
	Bucket *firstBucket;
	Bucket *lastBucket;
	size_t entriesPerBucket;

	Bucket *AllocBucket() {
		fplAssert(entriesPerBucket > 0);
		size_t size = sizeof(Bucket) + sizeof(uintptr_t) + sizeof(Entry) * entriesPerBucket;
		Bucket *result = (Bucket *)fplMemoryAllocate(size);
		result->firstFree = (Entry *)((uint8_t *)result + sizeof(Bucket) + sizeof(uintptr_t));
		for (size_t i = 0; i < entriesPerBucket - 1; ++i) {
			Entry *thisEntry = result->firstFree + i;
			Entry *nextEntry = result->firstFree + (i + 1);
			thisEntry->next = nextEntry;
		}
		return(result);
	}
public:
	static GrowablePool<T> Make(const size_t entriesPerBucket) {
		GrowablePool<T> result = {};
		result.entriesPerBucket = entriesPerBucket;
	}

	T *Aquire() {
		// Init
		if (entriesPerBucket == 0) {
			entriesPerBucket = 64;
		}

		Bucket *bucket;
		if (!lastBucket) {
			// Initial bucket
			bucket = AllocBucket();
			lastBucket = firstBucket = bucket;
		}
		if (lastBucket->firstFree == nullptr) {
			// Bucket full, add another
			bucket = AllocBucket();
			lastBucket->next = bucket;
			lastBucket = bucket;
		} else {
			bucket = lastBucket;
		}
		// Add entry
		fplAssert(bucket != nullptr);
		fplAssert(bucket->firstFree != nullptr);
		Entry *entry = bucket->firstFree;
		Entry *next = entry->next;
		bucket->firstFree = next;
		T *result = &entry->value;
		entry->next = nullptr;
		return(result);
	}

	void Release() {
		Bucket *bucket = firstBucket;
		while (bucket != nullptr) {
			Bucket *next = bucket->next;
			fplMemoryFree(bucket);
			bucket = next;
		}
		firstBucket = lastBucket = nullptr;
	}
};

template <typename T>
struct LinkedList {
private:
	struct LinkedItem {
		T value;
		LinkedItem *next;
	};

	GrowablePool<LinkedItem> pool;
	LinkedItem *first;
	LinkedItem *last;
	size_t count;
public:
	struct Iterator {
		LinkedItem *cur;
		size_t index;

		bool HasNext() const {
			bool result = cur != nullptr;
			return(result);
		}

		T *MoveNext() {
			if (cur != nullptr) {
				cur = cur->next;
				++index;
				return(&cur->value);
			}
			return(nullptr);
		}

		T *Value() {
			if (cur != nullptr) {
				return(&cur->value);
			}
			return(nullptr);
		}
	};

	struct ConstIterator {
		const LinkedItem *cur;
		size_t index;

		bool HasNext() const {
			bool result = cur != nullptr;
			return(result);
		}

		const T *MoveNext() {
			if (cur != nullptr) {
				cur = cur->next;
				++index;
				return(&cur->value);
			}
			return(nullptr);
		}

		const T *Value() const {
			if (cur != nullptr) {
				return(&cur->value);
			}
			return(nullptr);
		}
	};

	size_t Count() const {
		return count;
	}

	T *Add() {
		LinkedItem *item = pool.Aquire();
		if (last == nullptr) {
			last = first = item;
		} else {
			last->next = item;
			last = item;
		}
		++count;
		return(&item->value);
	}

	Iterator GetIterator() {
		Iterator result = { first };
		return(result);
	}

	ConstIterator GetConstIterator() const {
		ConstIterator result = { first };
		return(result);
	}

	void Release() {
		pool.Release();
	}
};

static char glErrorCodeBuffer[16];
static const char *GetGLErrorString(const GLenum err) {
	switch (err) {
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return "GL_INVALID_FRAMEBUFFER_OPERATION";
		default:
			if (_itoa_s(err, glErrorCodeBuffer, fplArrayCount(glErrorCodeBuffer), 10) == 0)
				return (const char *)glErrorCodeBuffer;
			else
				return "";
	}
}

static void CheckGLError() {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		const char *msg = GetGLErrorString(err);
		assert(!msg);
	}
}

struct String {
	const char *base;
	size_t len;

	String() {}
	String(const char *base) { this->base = base; this->len = 0; }
	String(const char *base, size_t len) { this->base = base; this->len = len; }
};

struct StringTable {
private:
	static constexpr size_t EntryPadding = 8;

	struct Entry {
		String str;
		size_t size;
		Entry *next;
	};

	Entry *first;
	Entry *last;
	size_t count;

	Entry *AllocEntry(const size_t size) {
		// @MEMORY(final): Use a memory pool instead
		Entry *entry = (Entry *)malloc(sizeof(Entry) + EntryPadding + size);
		*entry = {};
		entry->size = size;

		if (first == nullptr) {
			first = entry;
		}
		if (last != nullptr) {
			last->next = entry;
		}
		last = entry;

		++count;

		return(entry);
	}

	void ReleaseEntry(Entry *entry) {
		free(entry);
	}

public:
	String MakeString(size_t len) {
		Entry *entry = AllocEntry(len + 1);
		String *result = &entry->str;
		result->base = (char *)(uint8_t *)entry + sizeof(Entry) + EntryPadding;
		result->len = len + 1;
		return(*result);
	}

	const char *CopyString(const char *str, const size_t len) {
		String s = MakeString(len);
		fplCopyStringLen(str, len, (char *)s.base, len + 1);
		const char *result = s.base;
		return(result);
	}

	const char *CopyString(const char *str) {
		size_t len = fplGetStringLength(str);
		const char *result = CopyString(str, len);
		return(result);
	}

	const char *CopyString(const String &str) {
		size_t len;
		if (str.len == 0)
			len = fplGetStringLength(str.base);
		else
			len = str.len - 1;
		const char *result = CopyString(str.base, len);
		return(result);
	}

	void ReleaseAll() {
		Entry *p = first;
		while (p != nullptr) {
			Entry *n = p->next;
			ReleaseEntry(p);
			p = n;
		}
	}
};



struct FontID {
	const char *name;

	static FontID Make(StringTable &table, const char *name) {
		const char *nameCopy = table.CopyString(name); // @TODO(final): Intern the name, so we can just do a ptr match
		FontID result = { nameCopy };
		return(result);
	}
};

struct LoadedFont {
	struct Glyph {
		// Pixel coordinates: TR, TL, BL, BR ([0, W] [0, 0] [0, H] [W, H])
		Vec2f uv[4]; // In range of 0.0 to 1.0
		Vec2f offset[4]; // In range of -1.0 to 1.0
		float advance; // In range of -1.0 to 1.0
		uint32_t codePoint; // The unicode codepoint
	};

	FontID id;
	Glyph *glyphs;
	uint32_t minChar;
	uint32_t maxChar;
	uint32_t bitmapWidth;
	uint32_t bitmapHeight;
	GLuint textureId;
	float fontSize;
	float ascent;
	float descent;

	static b32 LoadFromMemory(LoadedFont *outFont, const uint8_t *fontData, const int fontIndex, const float fontSize, const uint32_t minChar, const uint32_t maxChar, const uint32_t minBitmapSize = 256, const uint32_t maxBitmapSize = 8192) {
		uint32_t charCount = (maxChar - minChar) + 1;

		int fontOffset = stbtt_GetFontOffsetForIndex(fontData, fontIndex);
		if (fontOffset < 0) {
			return(false);
		}

		stbtt_fontinfo fontInfo;
		if (!stbtt_InitFont(&fontInfo, fontData, fontOffset)) {
			return(false);
		}

		float pixelScale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);

		int ascent, descent, lineGap;
		stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

		stbtt_packedchar *packedChars = (stbtt_packedchar *)malloc(sizeof(stbtt_packedchar) * charCount);

		uint32_t bitmapSize = minBitmapSize;
		uint8_t *fontBitmap = nullptr;

		// @TODO(final): Support for multiple textures / codepoint-ranges, instead of finding the biggest bitmap

		// Find bitmap size where every character fits in
		b32 isPacked = false;
		do {
			fontBitmap = (uint8_t *)malloc(bitmapSize * bitmapSize);

			stbtt_pack_context context;
			stbtt_PackBegin(&context, fontBitmap, bitmapSize, bitmapSize, 0, 1, nullptr);

			int oversampleX = 2, oversampleY = 2;
			stbtt_PackSetOversampling(&context, oversampleX, oversampleY);

			if (stbtt_PackFontRange(&context, fontData, 0, fontSize, minChar, charCount, packedChars)) {
				isPacked = true;
			} else {
				free(fontBitmap);
				bitmapSize *= 2;
			}

			stbtt_PackEnd(&context);
		} while (!isPacked && (bitmapSize < maxBitmapSize));

		if (fontBitmap == nullptr) {
			free(packedChars);
			return(false);
		}

		float invAtlasW = 1.0f / (float)bitmapSize;
		float invAtlasH = 1.0f / (float)bitmapSize;

		float fontScale = 1.0f / fontSize;

		Glyph *glyphs = (Glyph *)fplMemoryAllocate(sizeof(Glyph) * charCount);

		for (uint32_t charIndex = 0; charIndex < charCount; ++charIndex) {
			const stbtt_packedchar *b = packedChars + charIndex;

			Glyph *outGlyph = glyphs + charIndex;

			outGlyph->codePoint = minChar + charIndex;

			float s0 = b->x0 * invAtlasW;
			float t0 = b->y0 * invAtlasH;
			float s1 = b->x1 * invAtlasW;
			float t1 = b->y1 * invAtlasH;

			float x0 = b->xoff * fontScale;
			float y0 = b->yoff * fontScale;
			float x1 = b->xoff2 * fontScale;
			float y1 = b->yoff2 * fontScale;

			outGlyph->offset[0] = V2f(x1, y0); // Top-right
			outGlyph->offset[1] = V2f(x0, y0); // Top-left
			outGlyph->offset[2] = V2f(x0, y1); // Bottom-left
			outGlyph->offset[3] = V2f(x1, y1); // Bottom-right

			outGlyph->uv[0] = V2f(s1, t0);
			outGlyph->uv[1] = V2f(s0, t0);
			outGlyph->uv[2] = V2f(s0, t1);
			outGlyph->uv[3] = V2f(s1, t1);

			outGlyph->advance = b->xadvance * fontScale;
		}

		free(packedChars);

		GLuint fontTexture;
		glGenTextures(1, &fontTexture);
		glBindTexture(GL_TEXTURE_2D, fontTexture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, bitmapSize, bitmapSize, 0, GL_ALPHA, GL_UNSIGNED_BYTE, fontBitmap);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, bitmapSize.w, bitmapSize.h, 0, GL_RED, GL_UNSIGNED_BYTE, fontBitmap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		CheckGLError();

		free(fontBitmap);

		LoadedFont font = fplZeroInit;
		font.fontSize = fontSize;
		font.glyphs = glyphs;
		font.minChar = minChar;
		font.maxChar = maxChar;
		font.bitmapWidth = bitmapSize;
		font.bitmapHeight = bitmapSize;
		font.textureId = fontTexture;
		font.descent = descent * pixelScale * fontScale;
		font.ascent = ascent * pixelScale * fontScale;

		*outFont = font;

		return(true);
	}

	static b32 LoadFromFile(LoadedFont *outFont, const char *filePath, const int fontIndex, const float fontSize, const uint32_t minChar, const uint32_t maxChar, const uint32_t minBitmapSize = 256, const uint32_t maxBitmapSize = 8192) {
		fplFileHandle fontFile;
		if (fplOpenBinaryFile(filePath, &fontFile)) {
			uint32_t fileSize = fplGetFileSizeFromHandle32(&fontFile);
			uint8_t *fontData = (uint8_t *)fplMemoryAllocate(fileSize);
			fplReadFileBlock32(&fontFile, fileSize, fontData, fileSize);
			fplCloseFile(&fontFile);
			b32 result = LoadFromMemory(outFont, fontData, fontIndex, fontSize, minChar, maxChar, minBitmapSize, maxBitmapSize);
			fplMemoryFree(fontData);
			return(result);
		} else {
			return(false);
		}
	}

	void Release() {
		if (textureId > 0) {
			glDeleteTextures(1, &textureId);
			textureId = 0;
		}
		if (glyphs != nullptr) {
			fplMemoryFree(glyphs);
		}
	}
};

enum class ImageResourceType {
	FPLLogo128x128 = 0,
};

struct ImageResource {
	const uint8_t *bytes;
	const char *name;
	const size_t length;
	ImageResourceType type;
};

namespace ImageResources {
	static ImageResource FPLLogo128x128 = { fplLogo128x128ImageData, "FPL Logo 128x128", fplLogo128x128ImageDataSize, ImageResourceType::FPLLogo128x128 };
}

struct ImageID {
	const char *name;
	size_t index;

	static ImageID Make(StringTable &table, const char *name, const size_t index) {
		const char *nameCopy = table.CopyString(name);
		ImageID result = { nameCopy, index };
		return(result);
	}
};

struct LoadedImage {
	ImageID id;
	uint32_t width;
	uint32_t height;
	GLuint textureId;

	static b32 LoadFromMemory(LoadedImage *outImage, const uint8_t *bytes, const size_t length) {
		int w, h, comp;
		stbi_uc *pixels = stbi_load_from_memory((const stbi_uc *)bytes, (int)length, &w, &h, &comp, 4);
		if (pixels == nullptr) {
			return(false);
		}

		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		CheckGLError();

		stbi_image_free(pixels);

		LoadedImage result = fplZeroInit;
		result.width = w;
		result.height = h;
		result.textureId = textureId;

		*outImage = result;

		return(true);
	}

	static b32 LoadFromFile(LoadedImage *outImage, const char *filePath) {
		fplFileHandle fontFile;
		if (fplOpenBinaryFile(filePath, &fontFile)) {
			uint32_t fileSize = fplGetFileSizeFromHandle32(&fontFile);
			uint8_t *bytes = (uint8_t *)fplMemoryAllocate(fileSize);
			fplReadFileBlock32(&fontFile, fileSize, bytes, fileSize);
			fplCloseFile(&fontFile);
			b32 result = LoadFromMemory(outImage, bytes, fileSize);
			fplMemoryFree(bytes);
			return(result);
		} else {
			return(false);
		}
	}

	void Release() {
		if (textureId > 0) {
			glDeleteTextures(1, &textureId);
			textureId = 0;
		}
	}
};

constexpr int MaxFontCount = 16;
constexpr int MaxImagesCount = 64;

struct Framebuffer {
	GLuint fbo;
	GLuint textures[2];
	int width;
	int height;

	void Init(const int width, const int height) {
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// RGBA-Texture
		glGenTextures(2, &textures[0]);

		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Depth-Texture
		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textures[1], 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		CheckGLError();

		this->width = width;
		this->height = height;
	}

	void Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);
		CheckGLError();
	}

	void Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void UpdateIfNeeded(const int newWidth, const int newHeight) {
		if (fbo == 0 || newWidth != width || newHeight != height) {
			Release();
			Init(newWidth, newHeight);
		}
	}

	void Release() {
		if (fbo) {
			glDeleteFramebuffers(1, &fbo);
			fbo = 0;
		}
		if (textures[0]) {
			glDeleteTextures(2, &textures[0]);
			textures[0] = textures[1] = 0;
		}
		width = height = 0;
	}
};

struct Renderer {
	LoadedFont fonts[MaxFontCount]; // First font is always the debug font
	LoadedImage images[MaxImagesCount];
	const LoadedFont *debugFont;
	StringTable *strings;
	size_t numFonts;
	size_t numImages;

	Framebuffer cubeFramebuffer;

	static int CompareFont(const void *oa, const void *ob) {
		const LoadedFont *a = (const LoadedFont *)oa;
		const LoadedFont *b = (const LoadedFont *)ob;

		int delta;

		// Font name
		delta = strcmp(a->id.name, b->id.name);
		if (delta != 0) {
			return(delta);
		}

		// Font-size
		float fontSizeDelta = a->fontSize - b->fontSize;
		if (fontSizeDelta < 0) delta = -1;
		else if (fontSizeDelta > 0) delta = 1;
		else delta = 0;
		if (delta != 0) {
			return(delta);
		}

		return(0);
	}

	const LoadedFont *AddFontFromResource(const FontResource &resource, const float fontSize, const uint32_t minChar = 32, const uint32_t maxChar = 255) {
		fplAssert(numFonts < fplArrayCount(fonts));
		LoadedFont *font = fonts + numFonts;
		if (!LoadedFont::LoadFromMemory(font, resource.data, 0, fontSize, minChar, maxChar)) {
			return {};
		}
		numFonts++;
		FontID id = FontID::Make(*strings, resource.name);
		font->id = id;
		qsort(fonts, numFonts, sizeof(fonts[0]), CompareFont);
		return(font);
	}

	const LoadedFont *AddFontFromFile(const char *filePath, const char *name, const float fontSize, const uint32_t minChar = 32, const uint32_t maxChar = 255) {
		fplAssert(numFonts < fplArrayCount(fonts));
		LoadedFont *font = fonts + numFonts;
		if (!LoadedFont::LoadFromFile(font, filePath, 0, fontSize, minChar, maxChar)) {
			return {};
		}
		numFonts++;
		FontID id = FontID::Make(*strings, name);
		font->id = id;
		qsort(fonts, numFonts, sizeof(fonts[0]), CompareFont);
		return(font);
	}

	inline const LoadedFont *FindFont(const char *name, const float fontSize = 0.0f) const {
		const LoadedFont *result = nullptr;

		// First: Try to find a font which is equal or greater
		for (size_t fontIndex = 0; fontIndex < numFonts; ++fontIndex) {
			const LoadedFont *font = fonts + fontIndex;
			if ((strcmp(font->id.name, name) == 0) && (fontSize == 0.0f || font->fontSize > fontSize)) {
				result = font;
				break;
			}
		}

		// Second: Find font which is smaller
		if (result == nullptr && numFonts > 0) {
			for (size_t fontIndex = numFonts - 1; fontIndex > 0; fontIndex--) {
				const LoadedFont *font = fonts + fontIndex;
				if ((strcmp(font->id.name, name) == 0) && font->fontSize <= fontSize) {
					result = font;
				}
			}
		}

		return(result);
	}

	const LoadedImage *AddImageFromResource(const ImageResource &resource) {
		fplAssert(numImages < fplArrayCount(images));
		LoadedImage *image = images + numImages;
		if (!LoadedImage::LoadFromMemory(image, resource.bytes, resource.length)) {
			return {};
		}
		ImageID id = ImageID::Make(*strings, resource.name, numImages++);
		image->id = id;
		return(image);
	}

	const LoadedImage *AddImageFromFile(const char *filePath) {
		fplAssert(numImages < fplArrayCount(images));
		LoadedImage *image = images + numImages;
		if (!LoadedImage::LoadFromFile(image, filePath)) {
			return {};
		}
		ImageID id = ImageID::Make(*strings, filePath, numImages++);
		image->id = id;
		return(image);
	}

	const LoadedImage *FindImage(const char *name) const {
		const LoadedImage *result = nullptr;

		for (size_t imageIndex = 0; imageIndex < numImages; ++imageIndex) {
			const LoadedImage *image = images + imageIndex;
			if (strcmp(image->id.name, name) == 0) {
				result = image;
				break;
			}
		}

		return(result);
	}

	void Release() {
		for (size_t imageIndex = 0; imageIndex < numImages; ++imageIndex) {
			LoadedImage *image = images + imageIndex;
			image->Release();
		}
		for (size_t fontIndex = 0; fontIndex < numFonts; ++fontIndex) {
			LoadedFont *font = fonts + fontIndex;
			font->Release();
		}
		cubeFramebuffer.Release();
		numImages = 0;
		numFonts = 0;
	}
};

typedef float(EasingFunction)(const float x);

struct Easing {
	EasingFunction *func;
};

// Based on https://easings.net
namespace Easings {
	static Easing Linear = { [](float x) { return x; } };

	static Easing EaseInSine = { [](float x) { return 1.0f - Cosine((x * Pi32) * 0.5f); } };
	static Easing EaseOutSine = { [](float x) { return Sine((x * Pi32) * 0.5f); } };
	static Easing EaseInOutSine = { [](float x) { return -(Cosine(Pi32 * x) - 1.0f) * 0.5f; } };

	static Easing EaseInQuad = { [](float x) { return x * x; } };
	static Easing EaseOutQuad = { [](float x) { return 1.0f - (1.0f - x) * (1.0f - x); } };
	static Easing EaseInOutQuad = { [](float x) { return x < 0.5f ? 2.0f * x * x : 1.0f - Power(-2.0f * x + 2.0f, 2.0f) * 0.5f; } };

	static Easing EaseInCube = { [](float x) { return x * x * x; } };
	static Easing EaseOutCube = { [](float x) { return 1.0f - Power(1.0f - x, 3.0f); } };
	static Easing EaseInOutCube = { [](float x) { return x < 0.5f ? 4.0f * x * x * x : 1.0f - Power(-2.0f * x + 2.0f, 3.0f) * 0.5f; } };

	static Easing EaseInQuart = { [](float x) { return x * x * x * x; } };
	static Easing EaseOutQuart = { [](float x) { return 1.0f - Power(1.0f - x, 4.0f); } };
	static Easing EaseInOutQuart = { [](float x) { return x < 0.5f ? 8.0f * x * x * x * x : 1.0f - Power(-2.0f * x + 2.0f, 4.0f) * 0.5f; } };

	static Easing EaseInQuint = { [](float x) { return x * x * x * x * x; } };
	static Easing EaseOutQuint = { [](float x) { return 1.0f - Power(1.0f - x, 5.0f); } };
	static Easing EaseInOutQuint = { [](float x) { return x < 0.5f ? 16.0f * x * x * x * x * x : 1.0f - Power(-2.0f * x + 2.0f, 5.0f) * 0.5f; } };

	static Easing EaseInExpo = { [](float x) { return x == 0.0f ? 0 : Power(2.0f, 10.0f * x - 10.0f); } };
	static Easing EaseOutExpo = { [](float x) { return x == 1.0f ? 1.0f : 1.0f - Power(2.0f, -10.0f * x); } };
	static Easing EaseInOutExpo = { [](float x) {
		return x == 0.0f
		  ? 0.0f
		  : x == 1.0f
		  ? 1.0f
		  : x < 0.5f ? Power(2.0f, 20.0f * x - 10.0f) * 0.5f
		  : (2.0f - Power(2.0f, -20.0f * x + 10.0f)) * 0.5f;
	} };

	static Easing EaseInCircle = { [](float x) { return 1.0f - SquareRoot(1.0f - Power(x, 2.0f)); } };
	static Easing EaseOutCircle = { [](float x) { return SquareRoot(1.0f - Power(x - 1.0f, 2.0f)); } };
	static Easing EaseInOutCircle = { [](float x) {
		return x < 0.5f
		  ? (1.0f - SquareRoot(1.0f - Power(2.0f * x, 2.0f))) * 0.5f
		  : (SquareRoot(1.0f - Power(-2.0f * x + 2.0f, 2.0f)) + 1.0f) * 0.5f;
	} };
};

enum class AnimationState: int32_t {
	Stopped = 0,
	Running,
	Done,
};

struct Animation {
	Easing easing;
	float duration;
	float currentTime;
	float currentAlpha;
	float startAlpha;
	float targetAlpha;
	AnimationState state;
	b32 reverse;

	b32 IsActive() const {
		b32 result = state == AnimationState::Running;
		return(result);
	}

	void RunToggle(const float maxDuration, const Easing &easing) {
		float ta;
		if (!reverse) {
			ta = 0.0f;
		} else {
			ta = 1.0f;
		}
		RunTo(ta, maxDuration, easing);
	}

	void RunTo(const float targetAlpha, const float maxDuration, const Easing &easing) {
		this->targetAlpha = targetAlpha;
		this->startAlpha = currentAlpha;

		float span;
		if (targetAlpha < currentAlpha) {
			reverse = true;
			span = currentAlpha - targetAlpha;
		} else {
			reverse = false;
			span = targetAlpha - currentAlpha;
		}

		this->duration = maxDuration * span;
		this->currentTime = 0.0f;

		this->state = AnimationState::Running;
	}

	void Stop() {
		this->state = AnimationState::Stopped;
	}

	void ResetAndStart(const float duration, const b32 reverse, const Easing &easing) {
		this->easing = easing;

		this->duration = duration;
		this->currentTime = 0.0f;

		this->currentAlpha = reverse ? 1.0f : 0.0f;
		this->startAlpha = this->currentAlpha;
		this->targetAlpha = reverse ? 0.0f : 1.0f;

		this->state = AnimationState::Running;
		this->reverse = reverse;
	}

	void Update(const float dt) {
		switch (state) {
			case AnimationState::Stopped:
				break;

			case AnimationState::Running:
			{
				currentTime += dt;
				float t = fplMin(currentTime, duration) / duration;
				currentAlpha = easing.func(ScalarLerp(startAlpha, t, targetAlpha));
				if (currentTime >= duration) {
					currentTime = duration;
					currentAlpha = easing.func(targetAlpha);
					state = AnimationState::Done;
				}
			} break;
		}
	}
};



struct Label {
	TextStyle style;
	Vec2f pos;
	const char *fontName;
	const char *text;
	float fontSize;
	HorizontalAlignment hAlign;
	VerticalAlignment vAlign;
};

struct RectStyle {
	Background background;
};

struct Rect {
	RectStyle style;
	Vec2f pos;
	Vec2f size;
};

struct ImageStyle {
	Background background;
};

struct Image {
	ImageStyle style;
	Vec2f pos;
	Vec2f size;
	const char *imageName;
};

enum class ElementType {
	None = 0,
	Label,
	Rect,
	Image,
};

struct Element {
	union {
		Label label;
		Rect rect;
		Image image;
	};
	ElementType type;
};

struct SlideVariables {
	const char *slideName;
	uint32_t slideNum;
	uint32_t slideCount;
};

struct Slide {
	LinkedList<Element> elements;
	SlideVariables vars;
	Background background;
	Vec2f size;
	StringTable *strings;
	const char *name;
	size_t numElements;

	Element *AddElement(const ElementType type) {
		Element *result = elements.Add();
		result->type = type;
		return(result);
	}

	Label *AddLabel(const String &text, const Vec2f &pos, const char *fontName, const float fontSize, const HorizontalAlignment hAlign = HorizontalAlignment::Left, const VerticalAlignment vAlign = VerticalAlignment::Top, const TextStyle &style = {}) {
		Element *element = AddElement(ElementType::Label);
		Label *result = &element->label;
		result->pos = pos;
		result->fontName = strings->CopyString(fontName);
		result->fontSize = fontSize;
		result->hAlign = hAlign;
		result->vAlign = vAlign;
		result->text = strings->CopyString(text);
		result->style = style;
		return(result);
	}

	Rect *AddRect(const Vec2f &pos, const Vec2f &size) {
		Element *element = AddElement(ElementType::Rect);
		Rect *result = &element->rect;
		result->pos = pos;
		result->size = size;
		return(result);
	}

	Image *AddImage(const Vec2f &pos, const Vec2f &size, const char *imageName) {
		Element *element = AddElement(ElementType::Image);
		Image *result = &element->image;
		result->pos = pos;
		result->size = size;
		result->imageName = strings->CopyString(imageName);
		return(result);
	}

	void Release() {
		elements.Release();
	}
};

struct Presentation {
	LinkedList<Slide> slides;
	Vec2f size;
	StringTable *strings;

	Slide *AddSlide(const Vec2f &size, const char *name) {
		Slide *result = slides.Add();
		result->strings = strings;
		result->size = size;
		result->name = strings->CopyString(name);
		return(result);
	}

	void Release() {
		auto it = slides.GetIterator();
		for (Slide *slide = it.Value(); it.HasNext(); slide = it.MoveNext())
			slide->Release();
		slides.Release();
	}
};

struct SlideTemplate {
	Vec2f size;
	const char *name;
};

struct PresentationTemplate {
	LinkedList<SlideTemplate> slides;
	Vec2f size;

	SlideTemplate *AddSlide(const SlideTemplate &slideTemplate) {
		SlideTemplate *result = slides.Add();
		*result = slideTemplate;
		return(result);
	}

	void Release() {
		slides.Release();
	}
};

struct PresentationState {
	Animation slideAnimation;
	Vec2f startOffset;
	Vec2f currentOffset;
	Vec2f targetOffset;
	Slide *activeSlide;
	int32_t activeSlideIndex;
};

constexpr float CubeRotationDuration = 1.0f; // Duration in seconds
constexpr float CubeRotationDelay = 3.0f; // Delay in seconds

constexpr float CubeRadius = 0.25f;
constexpr float PointRadius = 10.0f;
constexpr float PointDistance = CubeRadius * 1.5f;
constexpr float PointRotationSpeed = 15.0f; // Radians in seconds

struct App {
	Presentation presentation;
	Renderer renderer;
	PresentationState state;
	StringTable strings;

	RandomSeries entropy;

	Vec3f currentCubePos;
	Vec3f currentCubeVelocity;

	Vec3f pointPos;
	float pointRotation;
};

static Vec2f ComputeTextSize(const LoadedFont &font, const char *text, const size_t textLen, const float charHeight) {
	Vec2f result = fplZeroInit;
	if (text != nullptr) {
		float totalWidth = 0.0f;
		const char *p = text;
		while (*p) {
			size_t n = (p - text) + 1;
			if (n > textLen) break;
			uint32_t codePoint = (unsigned char)*p;
			if (codePoint >= font.minChar && codePoint <= font.maxChar) {
				uint32_t charIndex = codePoint - font.minChar;
				const LoadedFont::Glyph *glyph = font.glyphs + charIndex;
				Vec2f verts[] = {
					glyph->offset[0] * charHeight,
					glyph->offset[1] * charHeight,
					glyph->offset[2] * charHeight,
					glyph->offset[3] * charHeight,
				};
				Vec2f min = glyph->offset[0];
				Vec2f max = glyph->offset[0];
				for (int i = 1; i < 4; ++i) {
					min = V2fMin(min, verts[i]);
					max = V2fMax(max, verts[i]);
				}
				totalWidth += glyph->advance * charHeight;
			}
			++p;
		}
		result.x = totalWidth;
		result.y = charHeight;
	}
	return(result);
}

static Vec2f ComputeBoxOffset(const Vec2f &size, const HorizontalAlignment horizonzalAlign = HorizontalAlignment::Left, const VerticalAlignment verticalAlign = VerticalAlignment::Top) {
	Vec2f result = V2f(0, 0);
	if (verticalAlign == VerticalAlignment::Bottom) {
		result += V2f(0, -size.h);
	} else 	if (verticalAlign == VerticalAlignment::Middle) {
		result += V2f(0, -size.h * 0.5f);
	}
	if (horizonzalAlign == HorizontalAlignment::Right) {
		result += V2f(-size.w, 0);
	} else 	if (horizonzalAlign == HorizontalAlignment::Center) {
		result += V2f(-size.w * 0.5f, 0);
	}
	return(result);
}

static Vec2f ComputeTextOffset(const LoadedFont &font, const char *text, const size_t textLen, const float charHeight, const HorizontalAlignment horizonzalAlign = HorizontalAlignment::Left, const VerticalAlignment verticalAlign = VerticalAlignment::Top) {
	Vec2f size = ComputeTextSize(font, text, textLen, charHeight);
	Vec2f result = ComputeBoxOffset(size, horizonzalAlign, verticalAlign);
	return(result);
}

static void RenderTextQuads(const float x, const float y, const char *text, const size_t textLen, const float charHeight, const LoadedFont &font, const Vec4f &color) {
	if (text != nullptr) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, font.textureId);
		glColor4fv(&color.m[0]);
		glBegin(GL_QUADS);
		const char *p = text;
		Vec2f d = V2f(0, 0);
		Vec2f pos = V2f(x, y) + d;
		float scale = charHeight;
		while (*p) {
			uint32_t codePoint = (unsigned char)*p;
			if (codePoint >= font.minChar && codePoint <= font.maxChar) {
				uint32_t charIndex = codePoint - font.minChar;
				const LoadedFont::Glyph *glyph = font.glyphs + charIndex;

				Vec2f v0 = pos + glyph->offset[0] * scale;
				Vec2f v1 = pos + glyph->offset[1] * scale;
				Vec2f v2 = pos + glyph->offset[2] * scale;
				Vec2f v3 = pos + glyph->offset[3] * scale;

				glTexCoord2fv(&glyph->uv[0].m[0]); glVertex2fv(&v0.m[0]);
				glTexCoord2fv(&glyph->uv[1].m[0]); glVertex2fv(&v1.m[0]);
				glTexCoord2fv(&glyph->uv[2].m[0]); glVertex2fv(&v2.m[0]);
				glTexCoord2fv(&glyph->uv[3].m[0]); glVertex2fv(&v3.m[0]);

				pos += V2f(glyph->advance * scale, 0);
			}
			++p;
		}
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
}

static void RenderLine(const Vec2f &a, const Vec2f &b, const Vec4f &color, const float lineWidth = 1.0f) {
	glLineWidth(lineWidth);
	glColor4fv(&color.m[0]);
	glBegin(GL_LINES);
	glVertex2f(a.x, a.y);
	glVertex2f(b.x, b.y);
	glEnd();
	glLineWidth(1.0f);
}

static void RenderFilledQuad(const Vec2f &pos, const Vec2f &size, const Vec4f &color0, const Vec4f &color1, const BackgroundKind kind) {
	glBegin(GL_QUADS);
	if (kind == BackgroundKind::GradientHorizontal) {
		glColor4fv(&color1.m[0]); glVertex2f(pos.x + size.w, pos.y);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y + size.h);
		glColor4fv(&color1.m[0]); glVertex2f(pos.x + size.w, pos.y + size.h);
	} else if (kind == BackgroundKind::GradientVertical) {
		glColor4fv(&color0.m[0]); glVertex2f(pos.x + size.w, pos.y);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y);
		glColor4fv(&color1.m[0]); glVertex2f(pos.x, pos.y + size.h);
		glColor4fv(&color1.m[0]); glVertex2f(pos.x + size.w, pos.y + size.h);
	} else if (kind == BackgroundKind::HalfGradientHorizontal) {
		glColor4fv(&color1.m[0]); glVertex2f(pos.x + size.w * 0.5f, pos.y);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y + size.h);
		glColor4fv(&color1.m[0]); glVertex2f(pos.x + size.w * 0.5f, pos.y + size.h);

		glColor4fv(&color0.m[0]); glVertex2f(pos.x + size.w, pos.y);
		glColor4fv(&color1.m[0]); glVertex2f(pos.x + size.w * 0.5f, pos.y);
		glColor4fv(&color1.m[0]); glVertex2f(pos.x + size.w * 0.5f, pos.y + size.h);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x + size.w, pos.y + size.h);
	} else if (kind == BackgroundKind::HalfGradientVertical) {
		glColor4fv(&color0.m[0]); glVertex2f(pos.x + size.w, pos.y);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y);
		glColor4fv(&color1.m[0]); glVertex2f(pos.x, pos.y + size.h * 0.5f);
		glColor4fv(&color1.m[0]); glVertex2f(pos.x + size.w, pos.y + size.h * 0.5f);

		glColor4fv(&color1.m[0]); glVertex2f(pos.x + size.w, pos.y + size.h * 0.5f);
		glColor4fv(&color1.m[0]); glVertex2f(pos.x, pos.y + size.h * 0.5f);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y + size.h);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x + size.w, pos.y + size.h);
	} else {
		glColor4fv(&color0.m[0]); glVertex2f(pos.x + size.w, pos.y);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y + size.h);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x + size.w, pos.y + size.h);
	}
	glEnd();
}

static void RenderFilledQuad(const Vec2f &pos, const Vec2f &size, const Vec4f &color) {
	RenderFilledQuad(pos, size, color, V4fInit(0, 0, 0, 0), BackgroundKind::Solid);
}

static void RenderStrokedQuad(const Vec2f &pos, const Vec2f &size, const Vec4f &color, const float lineWidth = 1.0f) {
	glLineWidth(lineWidth);
	glColor4fv(&color.m[0]);
	glBegin(GL_LINE_LOOP);
	glVertex2f(pos.x + size.w, pos.y);
	glVertex2f(pos.x, pos.y);
	glVertex2f(pos.x, pos.y + size.h);
	glVertex2f(pos.x + size.w, pos.y + size.h);
	glEnd();
	glLineWidth(1.0f);
}

static const char *ResolveText(const SlideVariables &vars, const char *source, char *buffer, size_t maxBufferLen) {
	buffer[0] = 0;
	const char *result = buffer;
	const char *s = source;
	if (s != nullptr) {
		size_t bufIndex = 0;
		while (*s) {
			char c = *s;
			if (c == '%') {
				++s;
				size_t varLen = 0;
				const char *varName = s;
				while (*s && *s != '%') {
					++varLen;
					++s;
				}
				if (*s == '%') {
					++s;
					// % Escape
					if (varLen == 0) {
						buffer[bufIndex++] = '%';
					} else {
						size_t remainingBufLen = maxBufferLen - bufIndex;
						char *remainingStart = &buffer[bufIndex];
						if (strncmp("SLIDE_NUM", varName, varLen) == 0) {
							const char *t = fplS32ToString(vars.slideNum, remainingStart, remainingBufLen);
							if (t != nullptr) {
								size_t addedCount = fplGetStringLength(remainingStart);
								bufIndex += addedCount;
							}
						} else if (strncmp("SLIDE_COUNT", varName, varLen) == 0) {
							const char *t = fplS32ToString(vars.slideCount, remainingStart, remainingBufLen);
							if (t != nullptr) {
								size_t addedCount = fplGetStringLength(remainingStart);
								bufIndex += addedCount;
							}
						} else if (strncmp("SLIDE_NAME", varName, varLen) == 0) {
							const char *t = vars.slideName;
							if (t != nullptr) {
								fplStringAppend(t, remainingStart, remainingBufLen);
								size_t addedCount = fplGetStringLength(t);
								bufIndex += addedCount;
							}
						}
					}
				} else {
					// Unterminated
					break;
				}
			} else {
				buffer[bufIndex++] = c;
				++s;
			}
		}
		buffer[bufIndex] = 0;
	}
	return(result);
}

static Vec2f ComputeTextBlockSize(Renderer &renderer, Slide &slide, const char *text, const char *fontName, const float fontSize, const float lineHeight) {
	const LoadedFont *font = renderer.FindFont(fontName, fontSize);
	Vec2f result = V2f(0, 0);
	const char *p = text;
	const char *start = p;
	while (*p) {
		while (*p && *p != '\n') {
			++p;
		}
		const size_t len = p - start;

		Vec2f textSize = ComputeTextSize(*font, start, len, fontSize);
		result += V2f(0, lineHeight);
		result.w = fplMax(result.w, textSize.w);

		if (*p == 0)
			break;
		++p;
		start = p;
	}
	return(result);
}

static void RenderBackground(const Vec2f &pos, const Vec2f &size, const Background &background) {
	if (background.kind != BackgroundKind::None) {
		RenderFilledQuad(pos, size, background.primaryColor, background.secondaryColor, background.kind);
	}
}

static void RenderLabel(const LoadedFont &font, const Label &label, const SlideVariables &vars) {
	static char tmpBuffer[4096]; // @REPLACE(tspaete): Not great using a static buffer here, find a better approach

	const TextStyle &style = label.style;
	const char *text = ResolveText(vars, label.text, tmpBuffer, fplArrayCount(tmpBuffer));
	float charHeight = label.fontSize;
	size_t textLen = fplGetStringLength(label.text);
	Vec2f pos = label.pos;
	Vec2f size = ComputeTextSize(font, text, textLen, charHeight);
	Vec2f align = ComputeBoxOffset(size, label.hAlign, label.vAlign);
	Vec2f boxPos = pos + align;
	Vec2f textPos = boxPos + V2f(0, font.ascent * charHeight);

	// Background
	RenderBackground(boxPos, size, style.background);

	// Shadow
	if (style.drawShadow) {
		RenderTextQuads(textPos.x + style.shadowOffset.x, textPos.y + style.shadowOffset.y, text, textLen, charHeight, font, style.shadowColor);
	}

	// Foreground
	RenderTextQuads(textPos.x, textPos.y, text, textLen, charHeight, font, style.foregroundColor);

#if DRAW_TEXT_BOUNDS
	// Draw bounds
	RenderStrokedQuad(boxPos, size, V4fInit(1, 0, 0, 1), 1.0f);

	// Draw baseline
	Vec2f baseline = boxPos + V2f(0, size.h + font.descent * charHeight);
	Vec2f ascent = baseline + V2f(0, -font.ascent * charHeight);
	RenderLine(baseline, baseline + V2f(size.w, 0), V4fInit(0, 1, 0, 1), 2.0f);
	RenderLine(ascent, ascent + V2f(size.w, 0), V4fInit(0, 0, 1, 1), 2.0f);
#endif
}

static void RenderTextureQuad(const GLuint texture, const Vec2f &pos, const Vec2f &size, const Vec4f &color) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glColor4fv(&color.m[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(pos.x + size.w, pos.y);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(pos.x, pos.y);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(pos.x, pos.y + size.h);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(pos.x + size.w, pos.y + size.h);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

static void RenderImageQuad(const LoadedImage &renderImage, const Vec2f &pos, const Vec2f &size, const Vec4f &color) {
	RenderTextureQuad(renderImage.textureId, pos, size, color);
}

static void RenderPoint(const Vec3f &pos, const float radius, const Vec4f color) {
	glPointSize(radius * 2.0f);
	glColor4fv(&color.m[0]);
	glBegin(GL_POINTS);
	glVertex3fv(&pos.m[0]);
	glEnd();
	glPointSize(1.0f);
}

static void RenderCube(const float rw, const float rh, const float rd, const Vec4f colors[3]) {
	float left = -rw;
	float right = rw;

	float top = rh;
	float bottom = -rh;

	float far = -rd;
	float near = rd;

	glBegin(GL_QUADS);
	// Top face (y = 1.0f)
	// Green
	glColor4fv(&colors[0].m[0]);
	glVertex3f(right, top, far);
	glVertex3f(left, top, far);
	glVertex3f(left, top, near);
	glVertex3f(right, top, near);

	// Bottom face (y = -1.0f)
	// Orange
	glColor4fv(&colors[0].m[0]);
	glVertex3f(right, bottom, near);
	glVertex3f(left, bottom, near);
	glVertex3f(left, bottom, far);
	glVertex3f(right, bottom, far);

	// Front face  (z = 1.0f)
	// Red
	glColor4fv(&colors[1].m[0]);
	glVertex3f(right, top, near);
	glVertex3f(left, top, near);
	glVertex3f(left, bottom, near);
	glVertex3f(right, bottom, near);

	// Back face (z = -1.0f)
	// Yellow
	glColor4fv(&colors[1].m[0]);
	glVertex3f(right, bottom, far);
	glVertex3f(left, bottom, far);
	glVertex3f(left, top, far);
	glVertex3f(right, top, far);

	// Left face (x = -1.0f)
	// Blue
	glColor4fv(&colors[2].m[0]);
	glVertex3f(left, top, near);
	glVertex3f(left, top, far);
	glVertex3f(left, bottom, far);
	glVertex3f(left, bottom, near);

	// Right face (x = 1.0f)
	// Magenta
	glColor4fv(&colors[2].m[0]);
	glVertex3f(right, top, far);
	glVertex3f(right, top, near);
	glVertex3f(right, bottom, near);
	glVertex3f(right, bottom, far);
	glEnd();
}

static void RenderImage(const LoadedImage &renderImage, const Image &image) {
	Vec2f pos = image.pos;
	Vec2f size = image.size;
	Vec2f align = ComputeBoxOffset(size, HorizontalAlignment::Left, VerticalAlignment::Top);
	Vec2f boxPos = pos + align;
	Vec2f imagePos = boxPos;
	const ImageStyle &style = image.style;

	// Background
	RenderBackground(boxPos, size, style.background);

	// Foreground
	RenderImageQuad(renderImage, imagePos, size, V4f(1, 1, 1, 1));

#if DRAW_IMAGE_BOUNDS
	// Draw bounds
	RenderStrokedQuad(boxPos, size, V4fInit(1, 0, 0, 1), 1.0f);
#endif
}

struct Viewport {
	int x, y;
	int w, h;
};

extern Viewport ComputeViewportByAspect(const Vec2i &screenSize, const float targetAspect) {
	int targetHeight = (int)(screenSize.w / targetAspect);
	Vec2i viewSize = V2iInit(screenSize.w, screenSize.h);
	Vec2i viewOffset = V2iInit(0, 0);
	if (targetHeight > screenSize.h) {
		viewSize.h = screenSize.h;
		viewSize.w = (int)(screenSize.h * targetAspect);
		viewOffset.x = (screenSize.w - viewSize.w) / 2;
	} else {
		viewSize.w = screenSize.w;
		viewSize.h = (int)(screenSize.w / targetAspect);
		viewOffset.y = (screenSize.h - viewSize.h) / 2;
	}
	Viewport result = { viewOffset.x, viewOffset.y, viewSize.w, viewSize.h };
	return(result);
}

static void UpdateFrame(App &app, const float dt) {
	PresentationState &state = app.state;

	//
	// Slide animation
	//
	Animation &slideAnim = state.slideAnimation;
	slideAnim.Update(dt);
	if (state.slideAnimation.IsActive()) {
		state.currentOffset = V2fLerp(state.startOffset, state.slideAnimation.currentAlpha, state.targetOffset);
	} else {
		state.currentOffset = state.targetOffset;
	}

	float pointRotScaleX = 0.01f;
	float pointRotScaleY = 0.025f;
	float pointRotScaleZ = -0.01f;
	app.pointPos = V3f(Cosine(app.pointRotation * pointRotScaleX) * PointDistance, -Sine(app.pointRotation * pointRotScaleY) * PointDistance, Sine(app.pointRotation * pointRotScaleZ) * PointDistance);

	Vec3f direction = app.pointPos;
	const float acceleration = 10.0f;
	app.currentCubeVelocity += direction * acceleration * dt;
	app.currentCubePos += app.currentCubeVelocity * dt;
	app.currentCubeVelocity *= 0.95f;
}

static void RenderSlide(const Slide &slide, const Renderer &renderer) {
	float w = slide.size.w;
	float h = slide.size.h;
	Vec2f radius = V2f(w, h) * 0.5f;
	Vec2f center = radius;

#if DRAW_SLIDE_CENTER
	RenderLine(center - V2f(radius.w, 0), center + V2f(radius.w, 0), V4fInit(0.5f, 0.5f, 0.5f, 1.0f), 1.0f);
	RenderLine(center - V2f(0, radius.h), center + V2f(0, radius.h), V4fInit(0.5f, 0.5f, 0.5f, 1.0f), 1.0f);
#endif

	auto it = slide.elements.GetConstIterator();
	for (const Element *element = it.Value(); it.HasNext(); element = it.MoveNext()) {
		switch (element->type) {
			case ElementType::Rect:
			{
				const Rect &rect = element->rect;
				RenderBackground(rect.pos, rect.size, rect.style.background);
			} break;

			case ElementType::Label:
			{
				const Label &label = element->label;
				const char *fontName = label.fontName;
				const LoadedFont *font = renderer.FindFont(fontName, label.fontSize);
				if (font != nullptr) {
					RenderLabel(*font, label, slide.vars);
				}
			} break;

			case ElementType::Image:
			{
				const Image &image = element->image;
				const char *imageName = image.imageName;
				const LoadedImage *renderImage = renderer.FindImage(imageName);
				if (renderImage != nullptr) {
					RenderImage(*renderImage, image);
				}
			} break;
		}
	}
}

static void RenderFrame(App &app, const Vec2i &winSize) {
	const PresentationState &state = app.state;
	const Presentation &presentation = app.presentation;
	Renderer &renderer = app.renderer;

	const LoadedFont *debugFont = app.renderer.debugFont;
	fplAssert(debugFont != nullptr);
	const float debugFontSize = 30.0f;

	const Slide *activeSlide = state.activeSlide;
	if (activeSlide == nullptr) {
		float w = 1280.0;
		float h = 720.0f;

		glViewport(0, 0, winSize.w, winSize.h);

		Mat4f proj = Mat4OrthoRH(0.0f, w, h, 0.0f, -1.0f, 1.0f);
		glLoadMatrixf(&proj.m[0]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const char *text = "No slide found!";
		size_t textLen = fplGetStringLength(text);
		Vec2f size = ComputeTextSize(*debugFont, text, textLen, debugFontSize);
		Vec2f offset = ComputeBoxOffset(size, HorizontalAlignment::Center, VerticalAlignment::Middle);
		RenderTextQuads(w * 0.5f + offset.x, h * 0.5f + offset.y, text, textLen, debugFontSize, *debugFont, V4f(1, 0, 0, 1));
	} else {
		float w = activeSlide->size.w;
		float h = activeSlide->size.h;
		float aspect = w / h;
		Vec2f center = V2f(w, h) * 0.5f;
		Mat4f orthoProj = Mat4OrthoRH(0.0f, w, h, 0.0f, -1.0f, 1.0f);
		Mat4f perspectiveProj = Mat4PerspectiveRH(DegreesToRadians(45.0f), aspect, 0.01f, 1000.0f);

		Viewport viewport = ComputeViewportByAspect(winSize, aspect);

#if USE_LETTERBOX_VIEWPORT
		glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
		glScissor(viewport.x, viewport.y, viewport.w, viewport.h);
		renderer.cubeFramebuffer.UpdateIfNeeded(viewport.w, viewport.h);
#else
		glViewport(0, 0, winSize.w, winSize.h);
		glScissor(0, 0, winSize.w, winSize.h);
		renderer.cubeFramebuffer.UpdateIfNeeded(winSize.w, winSize.h);
#endif

		float zoom = 1.0f;
		Mat4f scale = Mat4Scale(V3f(zoom, zoom, zoom));
		Mat4f view = Mat4Translation(V2f(w * 0.5f, h * 0.5f)) * scale;
		Vec2f zoomOffset = V2f(-w * 0.5f, -h * 0.5f);

		//
		// Cube
		//
#if DRAW_ROTATING_CUBE
		{
			fplAssert(winSize.w > 0 && winSize.h > 0);
			renderer.cubeFramebuffer.Bind();

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);

			Mat4f cubeTranslation = Mat4Translation(V3f(0, 0, -2.0f));
			Mat4f cubeView = cubeTranslation * Mat4LookAtRH(V3f(0, 0, 0), app.currentCubePos, V3f(0, 1, 0));
			Mat4f cubeMVP = perspectiveProj * cubeView;
			glLoadMatrixf(&cubeMVP.m[0]);

			Vec4f colors[3] = { V4f(1.0f, 0.0f, 0.0f, 1.0f), V4f(0.0f, 1.0f, 0.0f, 1.0f), V4f(0.0f, 0.0f, 1.0f, 1.0f) };
			RenderCube(CubeRadius, CubeRadius, CubeRadius, colors);

			glDisable(GL_CULL_FACE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(3.0f);
			RenderCube(CubeRadius * 1.25f, CubeRadius * 1.25f, CubeRadius * 1.25f, colors);
			glLineWidth(1.0f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_CULL_FACE);

#if 0
			Mat4f tempMat = perspectiveProj * cubeTranslation;
			glLoadMatrixf(&tempMat.m[0]);
			RenderPoint(app.pointPos, PointRadius, V4f(1, 1, 1, 1));
			RenderPoint(app.currentCubePos, PointRadius, V4f(1, 0, 0, 1));

			glLineWidth(2.0f);
			glColor4f(1, 1, 1, 1);
			glBegin(GL_LINES);
			glVertex3f(0, 0, 0);
			glVertex3fv(&app.pointPos.m[0]);
			glEnd();
			glLineWidth(1.0f);
#endif

			glDisable(GL_DEPTH_TEST);
			CheckGLError();

			renderer.cubeFramebuffer.Unbind();
		}
#endif // DRAW_ROTATING_CUBE

		glClearColor(0.1f, 0.2f, 0.3f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//
		// Slides
		//
		Vec2f slidePos = V2f(0, 0);
		auto it = presentation.slides.GetConstIterator();
		for (const Slide *slide = it.Value(); it.HasNext(); slide = it.MoveNext()) {
			Vec2f slideSize = slide->size;
			Mat4f slideModel = Mat4Translation(slidePos - state.currentOffset + zoomOffset);
			Mat4f slideMVP = orthoProj * view * slideModel;
			glLoadMatrixf(&slideMVP.m[0]);

			RenderBackground(V2f(0, 0), slideSize, slide->background);

#if DRAW_ROTATING_CUBE
			RenderTextureQuad(renderer.cubeFramebuffer.textures[0], V2f(0, 0), slideSize, V4f(1, 1, 1, 0.2f));
#endif

			glLoadMatrixf(&slideMVP.m[0]);
			RenderSlide(*slide, renderer);

			slidePos += V2f(slide->size.w, 0);
		}

#if DRAW_VIEW_CENTER
		glLoadMatrixf(&proj.m[0]);
		RenderLine(center + V2f(-w * 0.25f, 0), center + V2f(w * 0.25f, 0), V4f(1, 1, 1, 1));
		RenderLine(center + V2f(0, -h * 0.25f), center + V2f(0, h * 0.25f), V4f(1, 1, 1, 1));
#endif

#if 0
		{
			Mat4f x = orthoProj * view;
			glLoadMatrixf(&x.m[0]);
			static char buffer[100];
			fplFormatString(buffer, fplArrayCount(buffer), "Vel: %.4f, %.4f, %.4f", app.currentCubeVelocity.x, app.currentCubeVelocity.y, app.currentCubeVelocity.z);
			size_t textLen = fplGetStringLength(buffer);
			const char *text = buffer;
			RenderTextQuads(0.0f, 0.0f, text, textLen, 20.0f, *debugFont, V4f(1, 1, 1, 1));
		}
#endif

	}

	CheckGLError();
	glFlush();
}

static void ReleaseApp(App &app) {
	app.presentation.Release();
	app.renderer.Release();
	app.strings.ReleaseAll();
}

static Rect2f AddHeaderAndFooter(Slide *slide, const HeaderDefinition &headerDef, const FooterDefinition &footerDef) {
	const char *headerFontName = headerDef.font.name;
	const float headerFontSize = headerDef.font.size;
	const TextStyle &headerFontStyle = headerDef.font.style;
	float headerHeight = headerDef.height;
	Vec2f headerPadding = headerDef.padding;

	const char *footerFontName = footerDef.font.name;
	const float footerFontSize = footerDef.font.size;
	const TextStyle &footerFontStyle = footerDef.font.style;
	float footerHeight = footerDef.height;
	Vec2f footerPadding = footerDef.padding;

	float w = slide->size.w;
	float h = slide->size.h;

	Vec2f logoSize = V2f(32, 32);

	Rect *rectTop = slide->AddRect(V2f(0, 0), V2f(w, headerHeight));
	rectTop->style.background.primaryColor = RGBAToLinearRaw(119, 113, 197, 255);
	rectTop->style.background.secondaryColor = RGBAToLinearRaw(0, 0, 0, 255);
	rectTop->style.background.kind = BackgroundKind::GradientVertical;

	Label *fplLabelTopLeft = slide->AddLabel(headerDef.leftText, rectTop->pos + headerPadding, headerFontName, headerFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, headerFontStyle);

	Image *fplLogo = slide->AddImage(rectTop->pos + V2f(w - logoSize.w, 0), logoSize, ImageResources::FPLLogo128x128.name);

	Rect *rectBottom = slide->AddRect(V2f(0, h - footerHeight), V2f(w, footerHeight));
	rectBottom->style.background.primaryColor = RGBAToLinearRaw(0, 0, 0, 255);
	rectBottom->style.background.secondaryColor = RGBAToLinearRaw(119, 113, 197, 255);
	rectBottom->style.background.kind = BackgroundKind::GradientVertical;

	Label *fplLabelBottomLeft = slide->AddLabel(footerDef.leftText, rectBottom->pos + V2f(footerPadding.x, rectBottom->size.h - footerPadding.y), footerFontName, footerFontSize, HorizontalAlignment::Left, VerticalAlignment::Bottom, footerFontStyle);

	Label *fplLabelBottomCenter = slide->AddLabel(footerDef.centerText, rectBottom->pos + V2f(rectBottom->size.w * 0.5f, rectBottom->size.h - footerPadding.y), footerFontName, footerFontSize, HorizontalAlignment::Center, VerticalAlignment::Bottom, footerFontStyle);

	Label *fplLabelBottomRight = slide->AddLabel(footerDef.rightText, rectBottom->pos + V2f(w - footerPadding.x, rectBottom->size.h - footerPadding.y), footerFontName, footerFontSize, HorizontalAlignment::Right, VerticalAlignment::Bottom, footerFontStyle);

	Rect2f result = R2fInit(V2f(0, headerHeight), V2f(w, h - headerHeight * 2));
	return(result);
}

static void UpdateSlideVariables(const Presentation &presentation, Slide &slide, const uint32_t slideNum) {
	slide.vars = {};
	slide.vars.slideCount = (uint32_t)presentation.slides.Count();
	slide.vars.slideNum = slideNum;
	slide.vars.slideName = slide.name;
}

static void UpdatePresentationVariables(Presentation &presentation) {
	auto it = presentation.slides.GetIterator();
	for (Slide *slide = it.Value(); it.HasNext(); slide = it.MoveNext()) {
		UpdateSlideVariables(presentation, *slide, (uint32_t)(it.index + 1));
	}
}

static Vec2f GetSlidePositionForSlide(const Presentation &presentation, const uint32_t slideIndex, const Vec2f direction) {
	Vec2f result = V2f(0, 0);
	auto it = presentation.slides.GetConstIterator();
	for (const Slide *slide = it.Value(); it.HasNext(); slide = it.MoveNext()) {
		if (it.index == slideIndex) {
			break;
		}
		result += direction * V2fDot(slide->size, direction);
	}
	return(result);
}

static Slide *GetSlideFromIndex(Presentation &presentation, const uint32_t slideIndex) {
	Slide *result = nullptr;
	auto it = presentation.slides.GetIterator();
	for (Slide *slide = it.Value(); it.HasNext(); slide = it.MoveNext()) {
		if (it.index == slideIndex) {
			return(slide);
		}
	}
	return(nullptr);
}

static void ShowSlideshow(App &app, const uint32_t slideIndex, const bool withTransition) {
	size_t slideCount = app.presentation.slides.Count();
	if (slideCount > 0 && slideIndex < slideCount) {
		Slide *slide = GetSlideFromIndex(app.presentation, slideIndex);
		app.state.activeSlideIndex = slideIndex;
		app.state.activeSlide = slide;

		// Offset starts at the left of the first slide and goes to the middle to the left of the last slide
		Vec2f moveDir = V2f(1, 0);
		Vec2f targetSlidePos = GetSlidePositionForSlide(app.presentation, slideIndex, moveDir);

		if (withTransition) {
			app.state.targetOffset = targetSlidePos;
			app.state.startOffset = app.state.currentOffset;
			float duration;
			const float maxDuration = 1.0f;
			if (app.state.slideAnimation.IsActive()) {
				float remaining = fplMax(0, app.state.slideAnimation.duration - app.state.slideAnimation.currentTime);
				duration = fplMax(0, fplMin(maxDuration - remaining, maxDuration));
			} else {
				duration = maxDuration;
			}
			app.state.slideAnimation.ResetAndStart(duration, false, Easings::EaseInOutExpo);
		} else {
			app.state.slideAnimation.Stop();
			app.state.targetOffset = app.state.currentOffset = app.state.startOffset = targetSlidePos;
		}
	}
}

static void JumpToNextSlide(App &app) {
	PresentationState &state = app.state;
	size_t slideCount = app.presentation.slides.Count();
	if (slideCount > 0 && state.activeSlideIndex < (int32_t)(slideCount - 1)) {
		ShowSlideshow(app, state.activeSlideIndex + 1, true);
	}
}

static void JumpToPrevSlide(App &app) {
	PresentationState &state = app.state;
	size_t slideCount = app.presentation.slides.Count();
	if (slideCount > 0 && state.activeSlideIndex > 0) {
		ShowSlideshow(app, state.activeSlideIndex - 1, true);
	}
}



static void AddTextBlock(Renderer &renderer, Slide &slide, const Vec2f &offset, const char *text, const char *fontName, const float fontSize, const float lineHeight, const TextStyle &style, const HorizontalAlignment hAlign, const VerticalAlignment vAlign) {
	const LoadedFont *font = renderer.FindFont(fontName, fontSize);
	Vec2f pos = offset;
	const char *p = text;
	const char *start = p;
	while (*p) {
		while (*p && *p != '\n') {
			++p;
		}
		const size_t len = p - start;

		Vec2f textSize = ComputeTextSize(*font, start, len, fontSize);
		slide.AddLabel(String(start, len + 1), pos, fontName, fontSize, hAlign, vAlign, style);
		pos += V2f(0, lineHeight);

		if (*p == 0)
			break;
		++p;
		start = p;
	}
}

static void AddSlideFromDefinition(Renderer &renderer, Presentation &presentation, const SlideDefinition &inSlide, const PresentationDefinition &inPresentation) {
	const char *titleFontName = inPresentation.titleFont.name;
	const float titleFontSize = inPresentation.titleFont.size;
	const float titleLineHeight = inPresentation.titleFont.lineScale * inPresentation.titleFont.size;
	const TextStyle &titleStyle = inPresentation.titleFont.style;

	const char *normalFontName = inPresentation.normalFont.name;
	const float normalFontSize = inPresentation.normalFont.size;
	const float normalLineHeight = inPresentation.normalFont.lineScale * inPresentation.normalFont.size;
	const TextStyle &normalStyle = inPresentation.normalFont.style;

	const char *consoleFontName = inPresentation.consoleFont.name;
	const float consoleFontSize = inPresentation.consoleFont.size;
	const float consoleLineHeight = inPresentation.consoleFont.lineScale * inPresentation.consoleFont.size;
	const TextStyle &consoleStyle = inPresentation.consoleFont.style;

	const float padding = inPresentation.padding;

	Slide *slide = presentation.AddSlide(presentation.size, inSlide.name);
	slide->background = inPresentation.background;

	Rect2f area = AddHeaderAndFooter(slide, inPresentation.header, inPresentation.footer);

	// Title
	slide->AddLabel(slide->name, area.pos + V2f(area.size.w * 0.5f, 0), titleFontName, titleFontSize, HorizontalAlignment::Center, VerticalAlignment::Top, titleStyle);

	// Content
	{
		TextBlockDefinition block = inSlide.content;

		const char *text = block.text;
		Vec2f blockSize = ComputeTextBlockSize(renderer, *slide, text, normalFontName, normalFontSize, normalLineHeight);

		HorizontalAlignment textAlign = HorizontalAlignment::Left;
		VerticalAlignment vAlign = VerticalAlignment::Top;

		Vec2f blockPos = area.pos;
		if (block.hAlign == HorizontalAlignment::Center) {
			textAlign = HorizontalAlignment::Center;
			blockPos += V2f((area.size.w - blockSize.w) * 0.5f, 0);
		} else if (block.hAlign == HorizontalAlignment::Left) {
			blockPos += V2f(padding, 0);
		} else if (block.hAlign == HorizontalAlignment::Right) {
			blockPos -= V2f(padding, 0);
		}
		if (block.vAlign == VerticalAlignment::Middle) {
			blockPos += V2f(0, (area.size.h - blockSize.h) * 0.5f);
		} else if (block.vAlign == VerticalAlignment::Top) {
			blockPos += V2f(0, titleLineHeight);
		}

#if 0
		Rect *rect = slide->AddRect(blockPos, blockSize);
		rect->style.background.kind = BackgroundKind::Solid;
		rect->style.background.primaryColor = V4f(1, 0, 1, 1);
#endif

		Vec2f textPos = blockPos;
		if (textAlign == HorizontalAlignment::Center) {
			textPos += V2fHadamard(V2f(1, 0), blockSize * 0.5f);
		} else if (textAlign == HorizontalAlignment::Right) {
			textPos += V2fHadamard(V2f(1, 0), blockSize);
		}
		AddTextBlock(renderer, *slide, textPos, text, normalFontName, normalFontSize, normalLineHeight, normalStyle, textAlign, vAlign);
	}
}

static void BuildPresentation(const PresentationDefinition &inPresentation, Renderer &renderer, Presentation &outPresentation) {
	Vec2f slideSize = inPresentation.slideSize;
	outPresentation.size = slideSize;
	size_t slideCount = inPresentation.slideCount;
	for (size_t slideIndex = 0; slideIndex < slideCount; ++slideIndex) {
		const SlideDefinition &slideDef = inPresentation.slides[slideIndex];
		AddSlideFromDefinition(renderer, outPresentation, slideDef, inPresentation);
	}
}

static void QuaternionTests() {
}

int main(int argc, char **argv) {
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("FPL Demo | Presentation", settings.window.title, fplArrayCount(settings.window.title));
	settings.video.driver = fplVideoDriverType_OpenGL;
	settings.video.isVSync = true;
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
	if (fplPlatformInit(fplInitFlags_All, &settings)) {
		if (fglLoadOpenGL(true)) {
			glDisable(GL_DEPTH_TEST);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CCW);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_SCISSOR_TEST);

			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_POINT_SMOOTH);

			glDisable(GL_TEXTURE_2D);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			glMatrixMode(GL_MODELVIEW);

			glClearColor(0.1f, 0.2f, 0.3f, 1);

			App *appMemory = (App *)fplMemoryAllocate(sizeof(App));
			App &app = *appMemory;

			app.renderer.strings = &app.strings;
			app.presentation.strings = &app.strings;

			// First font is always the debug font
			app.renderer.debugFont = app.renderer.AddFontFromResource(FontResources::BitStreamVerySans, 16.0f);

			app.renderer.AddFontFromResource(FontResources::BitStreamVerySans, 32.0f);
			app.renderer.AddFontFromResource(FontResources::BitStreamVerySans, 48.0f);
			app.renderer.AddFontFromResource(FontResources::Arimo, 16.0f);
			app.renderer.AddFontFromResource(FontResources::Arimo, 32.0f);
			app.renderer.AddFontFromResource(FontResources::Arimo, 48.0f);

			app.renderer.AddImageFromResource(ImageResources::FPLLogo128x128);

#if 0
			app.renderer.AddFontFromFile("c:/windows/fonts/arial.ttf", "Arial", 24);
#endif

			BuildPresentation(FPLPresentation, app.renderer, app.presentation);

			UpdatePresentationVariables(app.presentation);

			ShowSlideshow(app, 0, false);

			app.currentCubePos = V3f(1, 0, 0) * PointDistance;
			app.currentCubeVelocity = V3f(0, 0, 0);

			app.entropy = RandomSeed(1337);

			float dt = 1.0f / 60.0f;
			double lastTime = fplGetTimeInSecondsHP();
			while (fplWindowUpdate()) {
				fplEvent ev;
				while (fplPollEvent(&ev)) {
					if (ev.type == fplEventType_Keyboard) {
						if (ev.keyboard.type == fplKeyboardEventType_Button) {
							if (ev.keyboard.buttonState == fplButtonState_Release) {
								switch (ev.keyboard.mappedKey) {
									case fplKey_F:
									{
										if (!fplIsWindowFullscreen()) {
											fplEnableWindowFullscreen();
										} else {
											fplDisableWindowFullscreen();
										}
									} break;

									case fplKey_PageUp:
										JumpToPrevSlide(app);
										break;

									case fplKey_PageDown:
										JumpToNextSlide(app);
										break;
								}
							}
						}
					}
				}

				if (!fplIsWindowRunning())
					break;

				fplWindowSize winSize = fplZeroInit;
				fplGetWindowSize(&winSize);
				fplAssert(winSize.width > 0 && winSize.height > 0);

				UpdateFrame(app, dt);

				RenderFrame(app, V2iInit(winSize.width, winSize.height));

				fplVideoFlip();

				double currentTime = fplGetTimeInSecondsHP();
				dt = (float)(currentTime - lastTime);
				app.pointRotation += dt * PointRotationSpeed;
				lastTime = currentTime;
			}

			if (fplIsWindowFullscreen()) {
				fplDisableWindowFullscreen();
			}

			ReleaseApp(app);

			fplMemoryFree(appMemory);
		}
		fplPlatformRelease();
	}
}