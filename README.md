# final_game_tech

This repository contains game/multimedia related libraries and utilities written in C / C++.

![alt text](https://github.com/f1nalspace/final_game_tech/blob/develop/assets/final_game_tech.jpg)

## Table of libraries

| Name                     | Description                                 | Platforms        | Language | Latest Version | State       |
|--------------------------|---------------------------------------------|------------------|----------|----------------|-------------|
| final_platform_layer.h   | Single file platform abstraction library    | Win32/Linux/Unix | C99      | 1.0.0          | Finished    |
| final_dynamic_opengl.h   | Single file opengl loader library           | Win32/Linux/Unix | C99      | 1.0.1          | Finished    |
| final_game_box.h         | Single file gameboy DMG emulator            | Independent      | C99      | 1.0.0          | Finished    |
| final_memory.h           | Single file heap memory handling library    | Independent      | C99      | 1.0.0          | Finished    |
| final_xml.h              | Single file xml parser library              | Independent      | C99      | 0.3.1-alpha    | Finished    |
| final_tiletrace.hpp      | Single file tilemap contour tracing library | Independent      | C++/11   | 1.02           | Finished    |

* All libraries written in C99 are fully C++ compatible.
* Few libraries are still in beta/alpha, even though they are finished.
* Only fully implemented, documented and well tested libraries will leave the alpha/beta phase.

## How to contribute

If you want to support any of this library in this repository, you can do this in the following way:

- Provide feedback by tickets (features, bugs, improvements, etc). 
- Provide new demos to the demos/ folder
- Improve existing demos by pull requests
- Test the demos/libraries in several configurations and share your feedback (Platform / Architecture / Compiler / IDE)
- Be active in the community, ask questions, help others
- Please feel free to spread the word

## Code Contribution

You are invited to provide pull quests at any time - helping to improve the libraries and/or demos for me or for others.

However, untested, sloppy generated or poorly designed or broken code will not be accepted!

### Code Quality Requirements
Code that you provide you must have a certain level of quality:
- Compiles without any errors
- Does not break existing libraries or demos
- Follows the same code style and namings of types, functions, defines, etc.
- Prevents collision of existing functions and types by using prefix and long names
- Good readable documentations of all defines, types and functions
- External library loading and handling supports runtime, static and dynamic linking
- Uses as less memory as possible and uses data-oriented structures
- Uses existing libraries from `demos/additions`, whenever possible

Also respect the following rules:
- Use `fplMemory*Allocate()` / `fplMemory*Free()` over malloc/free
- Use `final_memory.h`, when you do lots of memory allocations
- Use platform preprocessor guards, for platform specific code
- Prefer single translation units over multiple translation units
- Prefer C99 over C++/11

## AI Usage

Since 2026, libraries and demos are extensively improved using local and premium AI tools.
Every line of code that is/was generated or modified by AI is/was properly reviewed and tested by humans.

Positive examples are:
- PulseAudio backend for `final_platform_layer.h`
- PipeWire backend for `final_platform_layer.h`
- Critical audio bugfixes in `demos/additions/final_audio*.h`
- Fixing seek not working properly in `demos/FPL_FFMpeg`
- Fixing major audio issues in `final_game_box.h`
- Improving emulation processing in `final_game_box.h`

It is allowed to provide AI generated code by pull requests, but those must have a certain level of quality (See section `Code Contribution`).

## License

All libraries in the root folder of this repository are open-source and use the MIT-License.
Read `LICENSE` for more details.