# Final Game Tech Codebase

This repository contains several game/multimedia related libraries mostly written in C99.

Core library is the Final Platform Layer (FPL) library that contains various demo applications showcasing its capabilities.

## Project Structure

```
.
├── final_platform_layer.h          # Single-header-file platform abstraction library (C99)
├── final_dynamic_opengl.h          # Single-header-file OpenGL function loader (C99)
├── final_game_box.h                # Single-header-file DMG game boy emulator library (C99)
├── final_xml.h                     # Single-header-file simple XML parser library (C99)
├── final_memory.h                  # Single-header-file custom memory allocator (C99)
├── final_tiletrace.hpp             # Single-header-file contour tile tracing library (C++/11)
├── apps/
│   ├── EnumToSwitchConverter/      # C# GUI tool to convert enums to switch statements
│   ├── FontRendering/              # C++ font rendering tool using stb_truetype
│   ├── OpenGLExtParser/            # C++ parser for OpenGL extensions
│   ├── ProtParser/                 # C# prototype/function parser generator
│   ├── staticdatamaker/            # C tool for generating static data
│   └── Win32WindowTest/            # C++ Windows-specific window testing
├── demos/
│   ├── Final_Testbed/              # Playground for testing font loading and game platform
│   ├── FMEM_Test/                  # Tests for final_memory.h
│   ├── FOGL_Test/                  # Tests for final_dynamic_opengl.h
│   ├── FTT_TileTracingDemo/        # Demo for final_tiletrace.hpp
│   ├── FXML_Test/                  # Tests for final_xml.h
│   ├── Final_AudioTest/            # Audio format conversion testing
│   ├── FPL_AudioPlayer/            # Full-featured audio playback demo
│   ├── FPL_Console/                # Hello-World console application
│   ├── FPL_Crackout/               # Breakout-style game with Box2D
│   ├── FPL_DynamicLib_Client/      # Client using FPL as a dynamic library
│   ├── FPL_DynamicLib_Host/        # FPL compiled as a dynamic shared library
│   ├── FPL_Emulator/               # Game Boy DMG emulator
│   ├── FPL_FFMpeg/                 # FFmpeg integration demo
│   ├── FPL_GameTemplate/           # Game template demo
│   ├── FPL_ImageViewer/            # Image viewer with multi-threading
│   ├── FPL_ImGui/                  # ImGui integration demo
│   ├── FPL_Input/                  # Visual input display demo
│   ├── FPL_MiniAudio/              # miniaudio.h integration demo
│   ├── FPL_NBodySimulation/        # 2D fluid simulation with benchmarking
│   ├── FPL_NoCRT/                  # FPL without C Runtime Library (MSVC/Windows)
│   ├── FPL_NoPlatformIncludes/     # FPL without platform headers
│   ├── FPL_NoRuntimeLinking/       # FPL with static linking
│   ├── FPL_OpenGL/                 # OpenGL rendering demo
│   ├── FPL_Raytracer/              # Multi-threaded software raytracer
│   ├── FPL_SimpleAudio/            # Basic sine wave audio demo
│   ├── FPL_Software/               # Software rendering demo
│   ├── FPL_StaticLib_Client/       # Client linking against FPL static library
│   ├── FPL_StaticLib_Host/         # FPL compiled as a static library
│   ├── FPL_Test/                   # Unit tests for FPL functionality
│   ├── FPL_Towadev/                # Tower defence game demo
│   ├── FPL_Vulkan/                 # Vulkan rendering demo
│   ├── FPL_WaveAudio/              # Wave file audio playback
│   ├── FPL_Window/                 # Minimal window creation demo
│   └── additions/                  # Additional libraries and headers
│       ├── final_assets.h          # Asset management
│       ├── final_audio.h           # Audio playback interface
│       ├── final_audioconversion.h # Audio format conversion
│       ├── final_audiodemo.h       # Audio track loader
│       ├── final_audiosystem.h     # Audio system and mixer
│       ├── final_buffer.h          # Buffer management
│       ├── final_core.h            # Core definitions
│       ├── final_debug.h           # Debug utilities
│       ├── final_fontloader.h      # Font file loader
│       ├── final_fonts.h           # Embedded font data
│       ├── final_game.h            # API of the Game framework
│       ├── final_gameplatform.h    # Implementation of the Game framework
│       ├── final_geometry.h        # Geometric shapes and operations
│       ├── final_graphics.h        # Graphics utilities
│       ├── final_log.h             # Logging system
│       ├── final_math.h            # Vector math and geometric operations
│       ├── final_mp3loader.h       # MP3 file loader
│       ├── final_music.h           # Embedded music data
│       ├── final_opengl_render.h   # OpenGL rendering backend
│       ├── final_random.h          # Random number generation
│       ├── final_render.h          # Rendering system with command buffers
│       ├── final_utils.h           # General utilities
│       ├── final_vorbisloader.h    # Vorbis file loader
│       └── final_waveloader.h      # Wave file loader
```

## Core Library: final_platform_layer.h

The main platform abstraction layer that provides:
- Cross-platform window management
- Video/OpenGL rendering context setup
- Audio playback and capture
- Input handling (keyboard, mouse, controllers)
- File I/O and path operations
- Memory management utilities
- Threading and synchronization primitives

## Demo Applications

### Games

#### FPL_Crackout
A breakout-style game demonstrating:
- Physics with Box2D
- Game state management
- Rendering with Final Framework
- Input handling
- Audio integration

#### FPL_Towadev
A tower defence game that implements:
- Basic tower defence game code with full math
- Fully controlled by mouse
- Based on Final-Game-Framework
- Incomplete game, just two levels with developer-graphics

### FPL_GameTemplate
Simple template that can be used to start programming a game
- Uses Final Game Framework which already provides everything needed for a game

### Audio

#### FPL_AudioPlayer
Full-featured audio playback demo showcasing:
- Audio system integration
- Streaming of audio sources into a ring buffer
- Playback controls
- File format support (MP3, Vorbis, Wave)

#### FPL_SimpleAudio
Basic audio demo demonstrating the audio callback interface:
- Generates a sine wave tone
- Minimal example of FPL audio setup

#### FPL_WaveAudio
Wave file audio playback demo

### Apps/Simulations

#### FPL_Emulator
A Game Boy DMG emulator with:
- Full emulator functionality
- Debugger with disassembly
- Visual debugging features
- ROM loading from raw or zip files
- OpenGL rendering

#### FPL_ImageViewer
Image viewer app that loads and displays images:
- No user interface, everything is controlled by keyboard
- Uses multi-threading to preload multiple images at once
- Uses OpenGL for rendering with image filters to improve quality
- Uses stb_image.h for loading images into pixel buffers
- Has no proper downscale implementation

#### FPL_NBodySimulation
C++ 2D fluid simulation with different scenarios and an integrated benchmarking mode:
- Uses legacy OpenGL for rendering
- Makes heavy use of multi-threading

#### FPL_Raytracer
Multi-threaded 3D software raytracer:
- Inspired by "handmade ray" (Casey Muratori)
- Tests multi-threading and software video output
- Uses FPL software rendering backend

### Demos that use Third-Party Libraries

#### FPL_FFMpeg
FFmpeg integration demo showing:
- Packet caching & decoding
- Video/audio packet caching & decoding
- Uses OpenGL for rendering, but works also with fplVideoBackendType_Software
- Simple OSD that displays relevant infos: time, video/audio information
- Support for seeking, but not correctly implemented

#### FPL_ImGui
Example demo that has a full FPL implementation for ImGui

#### FPL_MiniAudio
Example demo that uses miniaudio.h in a FPL console application

### Graphic Backends

#### FPL_OpenGL
OpenGL rendering demo showcasing:
- How to set up OpenGL for legacy or modern rendering
- Renders a rotating quad with a triangle on top of it
- Uses a simple GLSL shader to render random pixels in the triangle (only in modern mode)

#### FPL_Software
Window demo that showcases software pixel manipulation and rendering:
- Setting up FPL to use the software rendering backend
- Support for resizing the backbuffer
- Filling the backbuffer with a moving rectangle and random noise generated pixels

#### FPL_Vulkan
Vulkan rendering demo demonstrating:
- Full or partial Vulkan initialization
- Support for validation layers
- Renders a blue background
- Planned to be extended with a simple rendering example identical to the FPL_OpenGL demo

### Input

#### FPL_Input
Visual demo that displays all held/pressed keys and buttons in a gamepad:
- Uses legacy OpenGL for rendering
- Allows switching between event or polling mode

### Platform & Linking Demos

#### FPL_Console
Hello-World console application demonstrating basic FPL console I/O

#### FPL_Window
Minimal demo showing window initialization and event handling

#### FPL_NoCRT
Demonstrates compiling FPL without the C Runtime Library (Windows/MSVC only):
- Bare-metal console application
- Custom vsnprintf implementation

#### FPL_NoPlatformIncludes
Demonstrates using FPL without including platform headers (windows.h, pthread.h, etc.):
- Uses opaque handles for platform abstractions

#### FPL_NoRuntimeLinking
Demonstrates FPL with static linking instead of runtime dynamic linking:
- Must link against POSIX/Win32 system libraries directly

#### FPL_DynamicLib_Host / FPL_DynamicLib_Client
Demonstrates compiling FPL as a shared dynamic library (DLL/SO) and consuming it from a client application

#### FPL_StaticLib_Host / FPL_StaticLib_Client
Demonstrates compiling FPL as a static library and linking it into a client application

### Test & Development

#### FPL_Test
Unit tests for FPL functionality (C++):
- Threading, synchronization primitives (mutexes, semaphores, condition variables)
- Memory operations
- String utilities

#### Final_Testbed
Playground for testing font loading and game platform components:
- Uses final_platform_layer.h, final_fontloader.h, final_gameplatform.h

#### Final_AudioTest
Audio format conversion testing:
- Comparison with miniaudio library
- Tests interleaved/deinterleaved sample conversion

#### FMEM_Test
Tests for final_memory.h:
- Memory blocks, temporary allocations, push operations

#### FXML_Test
Tests for final_xml.h:
- XML parsing with custom allocators

#### FOGL_Test
Tests for final_dynamic_opengl.h:
- Verifies the full OpenGL header and dynamic function loader

#### FTT_TileTracingDemo
Demonstrates final_tiletrace.hpp:
- Converts solid tilemaps into line segments for physics engines (e.g. Box2D)

## Additional Libraries

### Top-level Single-Header Libraries

- **final_platform_layer.h**: Cross-platform abstraction layer (window, audio, input, file I/O, threading)
- **final_dynamic_opengl.h**: OpenGL function loader with full OpenGL header
- **final_game_box.h**: DMG Game Boy emulator library
- **final_xml.h**: Simple XML parser library
- **final_memory.h**: Custom memory allocator with debugging features
- **final_tiletrace.hpp**: C++/11 contour tile tracing for solid tilemaps

### demos/additions/

Audio:
- **final_audio.h**: Audio playback interface
- **final_audioconversion.h**: Audio format conversion utilities
- **final_audiosystem.h**: Audio system and mixer
- **final_audiodemo.h**: Audio track/music loader
- **final_mp3loader.h**: MP3 file loader
- **final_vorbisloader.h**: Vorbis file loader
- **final_waveloader.h**: Wave file loader
- **final_music.h**: Embedded music data

Rendering:
- **final_render.h**: Rendering system with command buffers
- **final_opengl_render.h**: OpenGL rendering backend
- **final_graphics.h**: Graphics utilities
- **final_fonts.h**: Embedded font data
- **final_fontloader.h**: Font file loader

Game Framework:
- **final_game.h**: Game framework API
- **final_gameplatform.h**: Game framework implementation

Utilities:
- **final_math.h**: Vector math and geometric operations
- **final_geometry.h**: Geometric shapes and operations
- **final_buffer.h**: Buffer management
- **final_assets.h**: Asset management
- **final_core.h**: Core definitions
- **final_log.h**: Logging system
- **final_debug.h**: Debug utilities
- **final_random.h**: Random number generation
- **final_utils.h**: General utilities

## Utility Applications (apps/)

- **EnumToSwitchConverter**: C# GUI tool to convert enum definitions to switch/case statements
- **FontRendering**: C++ tool for font rendering using stb_truetype.h
- **OpenGLExtParser**: C++ parser that processes OpenGL extension specifications
- **ProtParser**: C# tool to parse and generate prototype/function declarations
- **staticdatamaker**: C tool for generating static data headers
- **Win32WindowTest**: C++ Windows-specific window API testing

## Key Design Patterns

1. **Single Header Files**: All core libraries are header-only with optional implementation
2. **Cross-Platform Abstraction**: Platform-specific code is conditionally compiled
3. **Modular Architecture**: Each demo focuses on specific capabilities while using shared libraries
4. **Simple Integration**: Easy to include and use in new projects

## Implementation Details

- Uses C99 standard for maximum portability
- Conditional compilation for platform-specific features
- Memory allocation macros that can be overridden
- Extensive use of inline functions for performance
- Comprehensive error handling and assertions
