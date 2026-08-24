/***
fgb_input_simulator.h - Scripted joypad input for automated testing of Final Game Box (FGB).

This is a standalone helper that sits on top of the public final_game_box.h joypad API
(fgbSetButtonState). The emulator core itself is NOT modified or required to change for this.

Define FGB_INPUT_SIMULATOR_IMPLEMENTATION in exactly one translation unit before including this file,
after final_game_box.h was included.

Script file format (plain text, one event per line, '#' starts a comment):

	<frame> down <button>              Press the button down at the given emulated frame
	<frame> up <button>                Release the button at the given emulated frame
	<frame> press <button> [frames]    Press at the given frame and release automatically
	                                   after [frames] frames (default: 5)

Buttons: start, select, a, b, up, down, left, right (case-insensitive).
Frame numbers are absolute emulated frame indexes starting at 0 for the first frame after power-on.
Events may be listed in any order; they are sorted by frame on load.

Example:

	# Skip the intro, then start the first level
	300 press start
	480 press a 10

See INPUT_SIMULATOR.md in the repository root for the full documentation.
***/

#ifndef FGB_INPUT_SIMULATOR_H
#define FGB_INPUT_SIMULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// The maximum number of button events a script may contain (a "press" line produces two events)
#define FGB_INPUT_SIM_MAX_EVENTS 4096

// The default number of frames a button stays down for a "press" line without an explicit hold duration
#define FGB_INPUT_SIM_DEFAULT_PRESS_HOLD_FRAMES 5

// A single scheduled button state change
typedef struct fgbInputSimEvent {
	// The absolute emulated frame index at which the event fires
	uint32_t frame;
	// The button, see fgbButtonType
	uint8_t button;
	// True when the button goes down, false when it is released
	bool isDown;
} fgbInputSimEvent;

// Holds a loaded input script and the replay position
typedef struct fgbInputSimulator {
	// All events sorted by frame
	fgbInputSimEvent events[FGB_INPUT_SIM_MAX_EVENTS];
	// The number of valid events
	size_t eventCount;
	// The number of events already applied to the system
	size_t appliedCount;
	// True when a script was loaded successfully
	bool isEnabled;
} fgbInputSimulator;

struct fgbSystem;

// Loads and parses the script at filePath into sim. Returns false on file or parse errors.
extern bool fgbInputSimLoadFromFile(fgbInputSimulator *sim, const char *filePath);
// Rewinds the replay position so the script can be applied again from frame 0 (e.g. after a ROM reload)
extern void fgbInputSimRestart(fgbInputSimulator *sim);
// Applies all not-yet-applied events with event.frame <= frameIndex to the system.
// Call once per emulated frame with a monotonically increasing frame index.
// Returns the number of events applied by this call.
extern uint32_t fgbInputSimApply(fgbInputSimulator *sim, struct fgbSystem *system, const uint32_t frameIndex);

#endif // FGB_INPUT_SIMULATOR_H

#if defined(FGB_INPUT_SIMULATOR_IMPLEMENTATION) && !defined(FGB_INPUT_SIMULATOR_IMPLEMENTED)
#define FGB_INPUT_SIMULATOR_IMPLEMENTED

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static bool fgbInputSim__ParseButton(const char *name, uint8_t *outButton) {
	static const char *buttonNames[] = { "start", "select", "a", "b", "up", "down", "left", "right" };
	const size_t buttonNameCount = sizeof(buttonNames) / sizeof(buttonNames[0]);
	for (size_t buttonIndex = 0; buttonIndex < buttonNameCount; ++buttonIndex) {
		if (strcasecmp(buttonNames[buttonIndex], name) == 0) {
			*outButton = (uint8_t)(fgbButtonType_Start + buttonIndex);
			return true;
		}
	}
	return false;
}

static bool fgbInputSim__AppendEvent(fgbInputSimulator *sim, const uint32_t frame, const uint8_t button, const bool isDown) {
	if (sim->eventCount >= FGB_INPUT_SIM_MAX_EVENTS) {
		return false;
	}
	fgbInputSimEvent *event = &sim->events[sim->eventCount++];
	event->frame = frame;
	event->button = button;
	event->isDown = isDown;
	return true;
}

static void fgbInputSim__SortEventsByFrame(fgbInputSimulator *sim) {
	// Stable insertion sort, scripts are small and mostly sorted already
	for (size_t sortIndex = 1; sortIndex < sim->eventCount; ++sortIndex) {
		fgbInputSimEvent pending = sim->events[sortIndex];
		size_t insertIndex = sortIndex;
		while (insertIndex > 0 && sim->events[insertIndex - 1].frame > pending.frame) {
			sim->events[insertIndex] = sim->events[insertIndex - 1];
			--insertIndex;
		}
		sim->events[insertIndex] = pending;
	}
}

extern bool fgbInputSimLoadFromFile(fgbInputSimulator *sim, const char *filePath) {
	memset(sim, 0, sizeof(*sim));

	FILE *file = fopen(filePath, "rb");
	if (file == NULL) {
		return false;
	}

	char lineBuffer[256];
	uint32_t lineNumber = 0;
	bool hasError = false;

	while (fgets(lineBuffer, sizeof(lineBuffer), file) != NULL) {
		++lineNumber;

		// Strip a trailing comment, then tokenize
		char *commentStart = strchr(lineBuffer, '#');
		if (commentStart != NULL) {
			*commentStart = 0;
		}

		const char *separators = " \t\r\n";
		char *frameToken = strtok(lineBuffer, separators);
		if (frameToken == NULL) {
			continue; // Empty or comment-only line
		}
		char *actionToken = strtok(NULL, separators);
		char *buttonToken = strtok(NULL, separators);
		char *holdToken = strtok(NULL, separators);

		if (actionToken == NULL || buttonToken == NULL) {
			fprintf(stderr, "InputSim: %s:%u: expected '<frame> <down|up|press> <button>'\n", filePath, lineNumber);
			hasError = true;
			break;
		}

		char *frameEnd = NULL;
		const unsigned long frameValue = strtoul(frameToken, &frameEnd, 10);
		if (frameEnd == frameToken || *frameEnd != 0) {
			fprintf(stderr, "InputSim: %s:%u: invalid frame number '%s'\n", filePath, lineNumber, frameToken);
			hasError = true;
			break;
		}
		const uint32_t frame = (uint32_t)frameValue;

		uint8_t button = 0;
		if (!fgbInputSim__ParseButton(buttonToken, &button)) {
			fprintf(stderr, "InputSim: %s:%u: unknown button '%s'\n", filePath, lineNumber, buttonToken);
			hasError = true;
			break;
		}

		bool appended;
		if (strcasecmp("down", actionToken) == 0) {
			appended = fgbInputSim__AppendEvent(sim, frame, button, true);
		} else if (strcasecmp("up", actionToken) == 0) {
			appended = fgbInputSim__AppendEvent(sim, frame, button, false);
		} else if (strcasecmp("press", actionToken) == 0) {
			uint32_t holdFrames = FGB_INPUT_SIM_DEFAULT_PRESS_HOLD_FRAMES;
			if (holdToken != NULL) {
				char *holdEnd = NULL;
				const unsigned long holdValue = strtoul(holdToken, &holdEnd, 10);
				if (holdEnd == holdToken || *holdEnd != 0 || holdValue == 0) {
					fprintf(stderr, "InputSim: %s:%u: invalid hold frame count '%s'\n", filePath, lineNumber, holdToken);
					hasError = true;
					break;
				}
				holdFrames = (uint32_t)holdValue;
			}
			appended = fgbInputSim__AppendEvent(sim, frame, button, true);
			appended = appended && fgbInputSim__AppendEvent(sim, frame + holdFrames, button, false);
		} else {
			fprintf(stderr, "InputSim: %s:%u: unknown action '%s' (expected down, up or press)\n", filePath, lineNumber, actionToken);
			hasError = true;
			break;
		}

		if (!appended) {
			fprintf(stderr, "InputSim: %s:%u: too many events (max %d)\n", filePath, lineNumber, FGB_INPUT_SIM_MAX_EVENTS);
			hasError = true;
			break;
		}
	}

	fclose(file);

	if (hasError) {
		memset(sim, 0, sizeof(*sim));
		return false;
	}

	fgbInputSim__SortEventsByFrame(sim);
	sim->isEnabled = true;
	return true;
}

extern void fgbInputSimRestart(fgbInputSimulator *sim) {
	sim->appliedCount = 0;
}

extern uint32_t fgbInputSimApply(fgbInputSimulator *sim, struct fgbSystem *system, const uint32_t frameIndex) {
	if (!sim->isEnabled) {
		return 0;
	}
	uint32_t appliedNow = 0;
	while (sim->appliedCount < sim->eventCount && sim->events[sim->appliedCount].frame <= frameIndex) {
		const fgbInputSimEvent *event = &sim->events[sim->appliedCount++];
		fgbSetButtonState(system, (fgbButtonType)event->button, event->isDown);
		++appliedNow;
	}
	return appliedNow;
}

#endif // FGB_INPUT_SIMULATOR_IMPLEMENTATION
