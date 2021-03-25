#ifndef APP_H
#define APP_H

#include <final_platform_layer.h>
#include <string>
#include <vector>

#include "base.h"
#include "render.h"
#include "font.h"

#include "demo1.h"
#include "demo2.h"
#include "demo3.h"
#include "demo4.h"

const int kWindowWidth = 1280;
const int kWindowHeight = 720;
const char *kAppVersion = "1.4.3";

#define VERY_SHORT_BENCHMARK 1

#if !VERY_SHORT_BENCHMARK
const size_t kBenchmarkFrameCount = 50;
const size_t kBenchmarkIterationCount = 10;
#else
const size_t kBenchmarkFrameCount = 4;
const size_t kBenchmarkIterationCount = 8;
#endif
const size_t kDemoCount = 4;

struct Window {
	int left, top;
	int width, height;

	Window();

	inline int GetLeft() {
		return left;
	}
	inline int GetTop() {
		return top;
	}
	inline int GetWidth() {
		return width;
	}
	inline int GetHeight() {
		return height;
	}
};

struct Application {
	std::string cpuName;
	Window  *window;
	Render::CommandBuffer *commandBuffer;

	Application();
	virtual ~Application();

	inline Window *GetWindow() {
		return window;
	}

	void Resize(const int width, const int height);

	virtual void Init() = 0;
	virtual void KeyDown(const fplKey key) = 0;
	virtual void KeyUp(const fplKey key) = 0;
	virtual void UpdateAndRender(const float frametime, const uint64_t cycles) = 0;
};

struct FrameStatistics {
	SPHStatistics stats;
	float simulationTime;

	FrameStatistics() {
		this->stats = SPHStatistics();
		this->simulationTime = 0.0f;
	}

	FrameStatistics(const SPHStatistics &stats, const float simulationTime) {
		this->stats = stats;
		this->simulationTime = simulationTime;
	}
};

struct BenchmarkIteration {
	std::vector<FrameStatistics> frames;

	BenchmarkIteration(const size_t maxFrames) {
		frames.reserve(maxFrames);
	}
};

struct DemoStatistics {
	size_t demoIndex;
	size_t scenarioIndex;
	size_t frameCount;
	size_t iterationCount;
	FrameStatistics min;
	FrameStatistics max;
	FrameStatistics avg;
};

struct OSDState {
	float x;
	float y;
	float fontHeight;
	Font *font;
	Render::TextureHandle texture;
};

struct DemoApplication : public Application {
	std::string demoTitle;
	bool benchmarkActive;
	bool benchmarkDone;
	std::vector<BenchmarkIteration> benchmarkIterations;
	BenchmarkIteration *activeBenchmarkIteration;
	size_t benchmarkFrameCount;
	int keyStates[256];

	std::vector<DemoStatistics> demoStats;

	size_t demoIndex;
	BaseSimulation *demo;

	bool simulationActive;
	size_t activeScenarioIndex;
	std::string activeScenarioName;

	bool multiThreadingActive;

	Font osdFont;
	Render::TextureHandle osdFontTexture;
	Font chartFont;
	Render::TextureHandle chartFontTexture;

	void LoadDemo(const size_t demoIndex);

	void PushDemoStatistics();
	void StartBenchmark();
	void StopBenchmark();

	void DrawOSDLine(OSDState *osdState, const char *str);

	void RenderBenchmark(OSDState *osdState, const float left, float bottom, const float width, const float height);

	DemoApplication();
	~DemoApplication();
	void Init();
	void UpdateAndRender(const float frameTime, const uint64_t cycles);
	void KeyUp(const fplKey key);
	void KeyDown(const fplKey key);
	void LoadScenario(size_t scenarioIndex);
};

#endif
