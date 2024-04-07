#ifndef PRESENTATION_H
#define PRESENTATION_H

#include <final_platform_layer.h>

#include <final_math.h>

#include "types.h"
#include "fonts.h"

struct SoundResource {
	const char* name;
	const char* relativeFilePath;

	static SoundResource CreateFromFile(const char* filePath) {
		const char* filename = fplExtractFileName(filePath);
		return { filename, filePath };
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

static SoundDefinition MakeSoundDef(const SoundResource& resource, const float startTime = 0.0f, const float targetDuration = FLT_MAX) {
	return MakeSoundDef(resource.name, startTime, targetDuration);
}

struct ImageResource {
	const char *name;
	const char *relativeFilePath;
	const uint8_t *bytes;
	size_t length;

	static ImageResource CreateFromMemory(const uint8_t *bytes, const char *name, const size_t length) {
		ImageResource result = {};
		result.relativeFilePath = nullptr;
		result.name = name;
		result.bytes = bytes;
		result.length = length;
		return result;
	}

	static ImageResource CreateFromFile(const char *filePath) {
		const char *filename = fplExtractFileName(filePath);
		ImageResource result = {};
		result.relativeFilePath = filePath;
		result.name = filename;
		return result;
	}
};

struct FontResource {
	const uint8_t *data;
	size_t size;
	const char *name;
};

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
	const ImageResource *imageResource;
	Vec2f size;
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
	BlockType type;
	BlockAlignment contentAlignment;
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

static BlockDefinition MakeImageDef(const Vec2f& pos, const Vec2f& size, BlockAlignment contentAlignment, const ImageResource *imageResource, const Vec2f& imageSize, const bool keepAspect) {
	BlockDefinition result = {};
	result.pos = pos;
	result.size = size;
	result.contentAlignment = contentAlignment;
	result.type = BlockType::Image;
	result.image.imageResource = imageResource;
	result.image.size = imageSize;
	result.image.keepAspect = keepAspect;
	return(result);
}

constexpr size_t MaxBlockCount = 16;
constexpr size_t MaxAudioSoundCount = 4;

struct SlideDefinition {
	const char* name;
	BlockDefinition blocks[MaxBlockCount];
	SoundDefinition sounds[MaxAudioSoundCount];
	BackgroundStyle background;
	Quaternion rotation;
	size_t blockCount;
	size_t soundCount;
	double autoTransitionInSeconds;
};

template<size_t N>
static SlideDefinition MakeSlideDef(const char* name, BlockDefinition(&blocks)[N], const BackgroundStyle& background, const Quaternion& rotation, const double autoTransitionInSeconds = 0.0) {
	fplAssert(N < MaxBlockCount);

	SlideDefinition result = {};
	result.name = name;
	result.background = background;
	for (size_t i = 0; i < N; ++i) {
		result.blocks[i] = blocks[i];
	}
	result.rotation = rotation;
	result.blockCount = N;
	result.autoTransitionInSeconds = autoTransitionInSeconds;
	return(result);
}

template<size_t NBlocks, size_t NSounds>
static SlideDefinition MakeSlideDef(const char* name, BlockDefinition(&blocks)[NBlocks], SoundDefinition(&sounds)[NSounds], const BackgroundStyle& background, const Quaternion& rotation, const double autoTransitionInSeconds = 0.0) {
	fplAssert(NBlocks < MaxBlockCount);
	fplAssert(NSounds < MaxAudioSoundCount);

	SlideDefinition result = {};
	result.name = name;
	result.background = background;
	for (size_t i = 0; i < NBlocks; ++i) {
		result.blocks[i] = blocks[i];
	}
	for (size_t i = 0; i < NSounds; ++i) {
		result.sounds[i] = sounds[i];
	}
	result.rotation = rotation;
	result.blockCount = NBlocks;
	result.soundCount = NSounds;
	result.autoTransitionInSeconds = autoTransitionInSeconds;
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
	size_t slideCount;
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
	const FontResource *fontResources;
	const SoundResource* soundResources;
	const SoundResource* imageResources;
	size_t fontResourceCount;
	size_t sourceResourceCount;
	size_t imageResourceCount;
};

fpl_extern void SerializePresentationToFile(const PresentationDefinition *definition, const char *filePath);
fpl_extern PresentationFile DeserializePresentationFromFile(const char* filePath);

#endif // PRESENTATION_H

#if (defined(PRESENTATION_IMPLEMENTATION) && !defined(PRESENTATION_IMPLEMENTED)) || FPL_IS_IDE
#define PRESENTATION_IMPLEMENTED

fpl_extern void SerializePresentationToFile(const PresentationDefinition* definition, const char* filePath) {

}

fpl_extern PresentationFile DeserializePresentationFromFile(const char* filePath) {
	PresentationFile result = fplZeroInit;
	return result;
}

#endif // PRESENTATION_IMPLEMENTATION