# Final Game Tech Codebase

C99/C17 and C++11 game/multimedia codebase built around single-header libraries and focused demos.

## Core Headers

- `final_platform_layer.h` — cross-platform window/video/audio/input/file/thread/time platform layer.
- `final_dynamic_opengl.h` — OpenGL loader/header.
- `final_game_box.h` — Game Boy emulator.
- `final_xml.h` — XML parser.
- `final_memory.h` — custom allocator.
- `final_tiletrace.hpp` — C++11 tilemap contour tracing.
- `final_ui.h` — renderer agnostic immediate mode user interface.
- `final_ui_texteditor.h` — code and text editor widget for `final_ui.h`.

## Layout

- `apps/` — utility/conversion tools.
- `demos/` — games, rendering/audio/input/platform demos, tests.
- `demos/additions/` — reusable helper headers for audio, rendering, game framework, math, geometry, assets, logging, utilities.

## Notes

- Header-only style with optional implementation macros.
- Cross-platform via conditional compilation.
- Demos are intentionally focused examples.

## Additional Resources

- README.txt