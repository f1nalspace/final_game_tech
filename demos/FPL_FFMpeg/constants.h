#pragma once

#include <final_platform_layer.h>

// Max number of frames in the queues
constexpr uint32_t MAX_VIDEO_FRAME_QUEUE_COUNT = 4;
constexpr uint32_t MAX_AUDIO_FRAME_QUEUE_COUNT = 8;
constexpr uint32_t MAX_FRAME_QUEUE_COUNT = fplMax(MAX_AUDIO_FRAME_QUEUE_COUNT, MAX_VIDEO_FRAME_QUEUE_COUNT);

// Total size of data from all packet queues
constexpr uint64_t MAX_PACKET_QUEUE_SIZE = fplMegaBytes(16);

// Min number of packet frames in a single queue
constexpr uint32_t MIN_PACKET_FRAMES = 25;

// External clock min/max frames
constexpr uint32_t EXTERNAL_CLOCK_MIN_FRAMES = 2;
constexpr uint32_t EXTERNAL_CLOCK_MAX_FRAMES = 10;

// External clock speed adjustment constants for realtime sources based on buffer fullness
constexpr double EXTERNAL_CLOCK_SPEED_MIN = 0.900;
constexpr double EXTERNAL_CLOCK_SPEED_MAX = 1.010;
constexpr double EXTERNAL_CLOCK_SPEED_STEP = 0.001;

// No AV sync correction is done if below the minimum AV sync threshold
constexpr double AV_SYNC_THRESHOLD_MIN = 0.04;
// No AV sync correction is done if above the maximum AV sync threshold
constexpr double AV_SYNC_THRESHOLD_MAX = 0.1;
// No AV correction is done if too big error
constexpr double AV_NOSYNC_THRESHOLD = 10.0;
// If a frame duration is longer than this, it will not be duplicated to compensate AV sync
constexpr double AV_SYNC_FRAMEDUP_THRESHOLD = 0.1;
// Default refresh rate of 1/sec
constexpr double DEFAULT_REFRESH_RATE = 0.01;
// Number of audio samples required to make an average.
constexpr int AV_AUDIO_DIFF_AVG_NB = 20;
// Maximum audio speed change to get correct sync
constexpr int AV_SAMPLE_CORRECTION_PERCENT_MAX = 10;
