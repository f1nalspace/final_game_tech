#ifndef PRESENTATION_H
#define PRESENTATION_H

#include <final_platform_layer.h>

#include <final_math.h>

#include "types.h"
#include "fonts.h"

enum class ResourceType : uintptr_t {
	None = 0,
	File,
	Memory
};

struct FileResource {
	const char *filePath;
};

struct MemoryResource {
	const uint8_t *data;
	size_t length;
};

struct Resource {
	const char *name;
	ResourceType type;

	union {
		FileResource file;
		MemoryResource memory;
	};

	static Resource CreateFromMemory(const char *name, const uint8_t *data, const size_t length) {
		Resource result = {};
		result.name = name;
		result.type = ResourceType::Memory;
		result.memory.data = data;
		result.memory.length = length;
		return result;
	}

	static Resource CreateFromFile(const char *name, const char *relativeFilePath) {
		Resource result = {};
		result.type = ResourceType::File;
		result.name = name;
		result.file.filePath = relativeFilePath;
		return result;
	}

	static Resource CreateFromFile(const char *relativeFilePath) {
		const char *filename = fplExtractFileName(relativeFilePath);
		return CreateFromFile(filename, relativeFilePath);
	}
};

struct SoundDefinition {
	const char* name;
	float startTime;
	float targetDuration;
};

static SoundDefinition MakeSoundDef(const char* name, const float startTime, const float targetDuration = FLT_MAX) {
	SoundDefinition result = {};
	result.name = name;
	result.startTime = startTime;
	result.targetDuration = targetDuration;
	return result;
}

static SoundDefinition MakeSoundDef(const Resource& resource, const float startTime = 0.0f, const float targetDuration = FLT_MAX) {
	return MakeSoundDef(resource.name, startTime, targetDuration);
}

enum class BlockType {
	None = 0,
	Text,
	Image,
};

struct TextBlockDefinition {
	Vec4f color;
	const char* text;
	float fontSize;
	HorizontalAlignment textAlign;
};

struct ImageBlockDefinition {
	Vec4f tintColor;
	Vec2f size;
	const Resource *imageResource;
	bool keepAspect;
};

struct BlockAlignment {
	HorizontalAlignment h;
	VerticalAlignment v;
};

static BlockAlignment MakeAlign(HorizontalAlignment h = HorizontalAlignment::Left, VerticalAlignment v = VerticalAlignment::Top) {
	BlockAlignment result = {};
	result.h = h;
	result.v = v;
	return(result);
}

struct BlockDefinition {
	Vec2f pos;
	Vec2f size;
	BlockAlignment contentAlignment;
	BlockType type;
	union {
		TextBlockDefinition text;
		ImageBlockDefinition image;
	};
};

static BlockDefinition MakeTextDef(const Vec2f& pos, const Vec2f& size, BlockAlignment contentAlignment, const char* text, const HorizontalAlignment textAlign = HorizontalAlignment::Left, const float fontSize = 0, const Vec4f& color = V4f(1, 1, 1, 1)) {
	BlockDefinition result = {};
	result.pos = pos;
	result.size = size;
	result.contentAlignment = contentAlignment;
	result.type = BlockType::Text;
	result.text.text = text;
	result.text.textAlign = textAlign;
	result.text.fontSize = fontSize;
	result.text.color = color;
	return(result);
}

static BlockDefinition MakeImageDef(const Vec2f& pos, const Vec2f& size, BlockAlignment contentAlignment, const Resource *imageResource, const Vec2f& imageSize, const bool keepAspect, const Vec4f &tintColor = V4f(1, 1, 1, 1)) {
	BlockDefinition result = {};
	result.pos = pos;
	result.size = size;
	result.contentAlignment = contentAlignment;
	result.type = BlockType::Image;
	result.image.imageResource = imageResource;
	result.image.size = imageSize;
	result.image.keepAspect = keepAspect;
	result.image.tintColor = tintColor;
	return(result);
}

struct SlideDefinition {
	const char* name;
	const char* talk;
	const BlockDefinition *blocks;
	const SoundDefinition *sounds;
	BackgroundStyle background;
	Quaternion rotation;
	uint32_t blockCount;
	uint32_t soundCount;
	double duration;
};

template<size_t N>
static SlideDefinition MakeSlideDef(const char* name, const BlockDefinition(&blocks)[N], const BackgroundStyle& background, const Quaternion& rotation, const double autoTransitionInSeconds = 0.0) {
	SlideDefinition result = {};
	result.name = name;
	result.background = background;
	result.blocks = blocks;
	result.blockCount = (uint32_t)N;
	result.rotation = rotation;
	result.duration = autoTransitionInSeconds;
	return(result);
}

template<size_t NBlocks, size_t NSounds>
static SlideDefinition MakeSlideDef(const char* name, const BlockDefinition(&blocks)[NBlocks], const SoundDefinition(&sounds)[NSounds], const BackgroundStyle& background, const Quaternion& rotation, const double autoTransitionInSeconds = 0.0) {
	SlideDefinition result = {};
	result.name = name;
	result.background = background;
	result.blocks = blocks;
	result.blockCount = (uint32_t)NBlocks;
	result.sounds = sounds;
	result.soundCount = (uint32_t)NSounds;
	result.rotation = rotation;
	result.duration = autoTransitionInSeconds;
	return(result);
}

struct FontDefinition {
	const char* name;
	float size;
	float lineScale;
	TextStyle style;
};

struct HeaderDefinition {
	FontDefinition font;
	float height;
	const char* leftText;
	const char* centerText;
	const char* rightText;
	Vec2f padding;
};

struct FooterDefinition {
	FontDefinition font;
	float height;
	const char* leftText;
	const char* centerText;
	const char* rightText;
	Vec2f padding;
};

struct PresentationDefinition {
	const SlideDefinition* slides;
	uint32_t slideCount;
	Vec2f slideSize;
	HeaderDefinition header;
	FooterDefinition footer;
	FontDefinition titleFont;
	FontDefinition normalFont;
	FontDefinition consoleFont;
	float padding;
};

struct PresentationFile {
	PresentationDefinition definition;
	const Resource *fonts;
	const Resource *sounds;
	const Resource *images;
	uint32_t fontCount;
	uint32_t soundCount;
	uint32_t imageCount;
};

#endif // PRESENTATION_H