// Lists the audio devices FPL enumerates per backend, plays nothing
#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO
#include <final_platform_layer.h>
#include <stdio.h>

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;
	const fplAudioBackendType backends[] = { fplAudioBackendType_PipeWire, fplAudioBackendType_PulseAudio, fplAudioBackendType_Alsa };
	for (size_t backendIndex = 0; backendIndex < fplArrayCount(backends); ++backendIndex) {
		fplSettings settings = fplMakeDefaultSettings();
		settings.audio.backend = backends[backendIndex];
		settings.audio.startAuto = false;
		settings.audio.stopAuto = false;
		const char *backendName = fplGetAudioBackendName(backends[backendIndex]);
		if (!fplPlatformInit(fplInitFlags_Audio, &settings)) {
			printf("%s: init failed\n", backendName);
			continue;
		}
		uint32_t deviceCount = fplGetAudioDevices(0, 0, fpl_null);
		fplAudioDeviceInfo *infos = (fplAudioDeviceInfo *)fplMemoryAllocate(sizeof(fplAudioDeviceInfo) * (deviceCount + 1));
		uint32_t loadedCount = fplGetAudioDevices(deviceCount, 0, infos);
		printf("%s: %u devices\n", backendName, loadedCount);
		for (uint32_t deviceIndex = 0; deviceIndex < loadedCount; ++deviceIndex) {
			printf("  [%u]%s %s\n", deviceIndex, infos[deviceIndex].isDefault ? " (default)" : "", infos[deviceIndex].name);
		}
		fplMemoryFree(infos);
		fplPlatformRelease();
	}
	return 0;
}
