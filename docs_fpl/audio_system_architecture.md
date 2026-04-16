# Audio System Architecture in Final Platform Layer (FPL)

## Overview

The audio system in FPL is designed to provide flexible, cross-platform audio playback support with multiple backend implementations. It is more complex than the video system due to the variety of audio APIs and the need for real-time audio streaming and device management.

## Architecture

### Backend Abstraction

- Audio backends are abstracted via a function table (`fplAudioBackendFunctionTable`) containing pointers to functions for:
  - Initialization and release of the backend.
  - Querying audio devices and device info.
  - Initializing and releasing audio devices.
  - Starting and stopping audio playback.
  - Running and stopping the audio main loop.

- The active audio backend is represented by a `fplAudioBackend` structure, which holds internal and desired audio formats, device info, and client callbacks for audio data.

- Backend-specific data is stored immediately after the `fplAudioBackend` structure with padding for alignment.

### Supported Backends

- **DirectSound** (Windows): Uses COM interfaces for audio playback.
- **ALSA** (Linux): Uses ALSA API for audio device management and playback.
- **Custom**: Allows user-defined audio backends.

### Audio Formats and Devices

- Audio formats are described by `fplAudioFormat` including sample rate, channels, buffer sizes, format type, and channel layout.
- Audio devices are represented by `fplAudioDeviceInfo` and extended info structures.
- Channel mappings and layouts are supported for multi-channel audio.

### Audio Playback Flow

- The audio system manages device states (uninitialized, stopped, started, starting, stopping).
- Audio data is provided via a client callback function that fills audio buffers.
- Playback can be synchronous or asynchronous depending on the backend.
- Worker threads may be used to handle audio streaming in asynchronous backends.