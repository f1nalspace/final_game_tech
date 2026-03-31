# Final Game Tech Codebase

This repository contains the Final Platform Layer (FPL) library and various demo applications showcasing its capabilities.

## Project Structure

```
.
├── final_platform_layer.h          # Core platform abstraction library
├── demos/
│   ├── FPL_AudioPlayer/            # Audio playback demo
│   ├── FPL_Crackout/               # Breakout-style game with Box2D
│   ├── FPL_Emulator/               # Game Boy DMG emulator
│   ├── FPL_FFMpeg/                 # FFmpeg integration demo
│   ├── FPL_Vulkan/                 # Vulkan rendering demo
│   ├── FPL_OpenGL/                 # OpenGL rendering demo
│   └── additions/                  # Additional libraries and headers
│       ├── final_game.h            # Game framework
│       ├── final_game_box.h        # Core game box components
│       ├── final_render.h          # Rendering system
│       ├── final_math.h            # Math utilities
│       ├── final_audiosystem.h     # Audio system
│       ├── final_memory.h          # Memory management
│       └── final_log.h             # Logging system
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

### FPL_Crackout
A breakout-style game demonstrating:
- Physics with Box2D
- Game state management
- Rendering with Final Framework
- Input handling
- Audio integration

### FPL_Emulator
A Game Boy DMG emulator with:
- Full emulator functionality
- Debugger with disassembly
- Visual debugging features
- ROM loading from raw or zip files
- OpenGL rendering

### FPL_AudioPlayer
Audio playback demo showcasing:
- Audio system integration
- Playback controls
- File format support

### FPL_FFMpeg
FFmpeg integration demo showing:
- Video/audio decoding
- Media processing capabilities

### FPL_Vulkan
Vulkan rendering demo demonstrating:
- Modern graphics API usage
- Shader compilation and loading
- Advanced rendering techniques

### FPL_OpenGL
OpenGL rendering demo showcasing:
- Basic and advanced OpenGL features
- Rendering pipeline setup
- Texture handling

## Additional Libraries

### additions/
- **final_game.h**: Game framework components for setting up games quickly
- **final_game_box.h**: Core game box structures and functions
- **final_render.h**: Rendering system with command buffers
- **final_math.h**: Vector math and geometric operations
- **final_audiosystem.h**: Audio system with playback capabilities
- **final_memory.h**: Custom memory allocator with debugging features
- **final_log.h**: Logging system for debugging and monitoring

## Key Design Patterns

1. **Single Header Files**: All core components are header-only with optional implementation
2. **Cross-Platform Abstraction**: Platform-specific code is conditionally compiled
3. **Modular Architecture**: Each demo focuses on specific capabilities while using shared libraries
4. **Simple Integration**: Easy to include and use in new projects

## Implementation Details

- Uses C99 standard for maximum portability
- Conditional compilation for platform-specific features
- Memory allocation macros that can be overridden
- Extensive use of inline functions for performance
- Comprehensive error handling and assertions