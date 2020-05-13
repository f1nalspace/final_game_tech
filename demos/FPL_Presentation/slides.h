#pragma once

#include "types.h"

struct TextBlockDefinition {
	const char *text;
	HorizontalAlignment hAlign;
	VerticalAlignment vAlign;

	TextBlockDefinition(const char *text, HorizontalAlignment hAlign, VerticalAlignment vAlign) {
		this->text = text;
		this->hAlign = hAlign;
		this->vAlign = vAlign;
	}

	TextBlockDefinition() : TextBlockDefinition(nullptr, HorizontalAlignment::Left, VerticalAlignment::Top) {
	}	
};

struct SlideDefinition {
	const char *name;
	TextBlockDefinition content;
};

static const SlideDefinition FPL_IntroSlide = {
	"Introduction",
	TextBlockDefinition((
		"Introducing Final-Platform-Layer (FPL).\n"
		"A lightweight Platform-Abstraction-Library written in C99.\n"
		"\n"
		"Created by Torsten Spaete, a professional software engineer with 20+ years of experience.\n"
	), HorizontalAlignment::Center, VerticalAlignment::Middle),
};

static const SlideDefinition FPL_WhatIsAPAL = {
	"What is a Platform-Abstraction-Library",
	TextBlockDefinition((
		"A Platform-Abstraction-Library (or short PAL) is a library written in a low-level language - like C,\n"
		"that abstracts low-level systems in a platform-independent way.\n"
		"\n"
		"This has the advantage of not having to deal with tons of platform/compiler specific implementation details,\n"
		"you have to deal with if you don´t use a PAL.\n"
	), HorizontalAlignment::Center, VerticalAlignment::Middle),
};

static const SlideDefinition FPL_WhatIsFPL = {
	"What is FPL",
	TextBlockDefinition((
		"FPL is an all-purpose / multimedia platform abstraction library,\n"
		"providing a powerful and easy to use API, accessing low-level systems in a platform-independent way:\n"
		"\n"
		"- Platform detection (x86/x64/Arm, Win32/Linux/Unix, etc.)\n"
		"- Compiler detection (MSVC/GCC/Clang/Intel)\n"
		"- Macros (Debugbreak, Assertions, CPU-Features, Memory init etc.)\n"
		"- Dynamic library loading (.dll/.so)\n"
		"- Single window creation and handling (Win32/X11)\n"
		"- Event and input polling (Keyboard/Mouse/Gamepad)\n"
		"- Video initialization and output (Software, OpenGL, etc.)\n"
		"- Asyncronous audio playback (DirectSound, ALSA, etc.)\n"
		"- IO (Console, Paths, Files, Directories, etc.)\n"
		"- Memory handling with or without alignment\n"
		"- Multithreading (Atomics, Threads, Mutexes, Semaphores, Conditionals, etc.)\n"
		"- Retrieving hardware informations\n"
		"- and many more\n"
	), HorizontalAlignment::Left, VerticalAlignment::Top),
};

static const SlideDefinition FPLSlides[] = {
	FPL_IntroSlide,
	FPL_WhatIsAPAL,
	FPL_WhatIsFPL,
};