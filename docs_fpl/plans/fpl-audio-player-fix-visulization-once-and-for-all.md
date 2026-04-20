# Context

I wrote a audio player that streams in audio samples and visualizes it in a few ways:
- As a series of quads that looks like a waveform
- As a single line
- As a FFT bars with half frequencies
- As spectrum peak bar

The audio samples are streamed into a ring buffer from the thread AudioStreamingThread.
The audio samples are played effectly by the thread callback AudioPlayback and samples are read from the ring buffer.

There is a realtime sample playback built-in (demo->useRealTimeSamples), which allows to overwrite the visualization->videoAudioChunks directly.
When this is enabled, then the visualized samples does not match the played/heared audio samples.

All audio computation and visualization happens in the `Render` function.

# Issues

1. FFT visualization wrong
The displayed FFT is simply not correct and even flickers a lot, even though i already interpolate the values.
Either the rendering is not correct the actual FFT calling is not correct or even the FFT math is wrong.

2. Spectrum Analyzer wrong
The spectrum visualizer is not correct at all and not even close to the real thing.
For example, when playing a sine wave, starting with a low frequency and interpolate to a high frequency, the bands should wander from left-to-right.
In my implementation this never happens and it always looks wrong - even though i also interpolate the bars for preventing extreme changes and flickering.

# Sources
- demos/FPL_AudioPlayer/fpl_audioplayer.c
- demos/additions/final_audio.h
- demos/additions/final_audiosystem.h
- demos/additions/final_audiodemo.h
- demos/additions/final_audioconversion.h

# Task
Create a plan for fixing issue 1 and 2.
Work only in the specified Sources i defined.
Split into two tasks, start with FFT because the spectrum analysis is based on the FFT data.
Store the plan as a markdown file in "docs_fpl/plans/fpl-audio-player-fix-visulization-once-and-for-all-plan.md".
