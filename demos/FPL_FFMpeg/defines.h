#pragma once

#define PRINT_QUEUE_INFOS 0
#define PRINT_FRAME_UPLOAD_INFOS 0
#define PRINT_MEMORY_STATS 0
#define PRINT_FRAME_DROPS 0
#define PRINT_VIDEO_REFRESH 0
#define PRINT_VIDEO_DELAY 0
#define PRINT_CLOCKS 0
#define PRINT_PTS 0
#define PRINT_FPS 0
#define PRINT_FLUSHES 0
#define PRINT_SEEKES 0

// Rendering mode (Hardware or Software)
#define USE_HARDWARE_RENDERING 1

// Hardware rendering
#define USE_GLSL_IMAGE_FORMAT_DECODING 1 // Use GLSL to decode image planes (Much faster than software decoding)
#define USE_GL_PBO 1 // Use OpenGL Pixel Buffer Objects (Faster CPU -> GPU transfer)
#define USE_GL_BLENDING 1 // Use OpenGL Blending (Only useful to disable, when debugging text rendering)
#define USE_GL_RECTANGLE_TEXTURES 1 // Use GL_TEXTURE_RECTANGLE instead of GL_TEXTURE_2D

// Software rendering
#define USE_FLIP_V_PICTURE_IN_SOFTWARE 0 // We need to detect this, when to flip and when not to flip

// Global
#define USE_FFMPEG_STATIC_LINKING 0 // Use static or runtime linking of FFMPEG (Useful to test if function signatures has been changed)
#define USE_FFMPEG_SOFTWARE_CONVERSION 0 // Convert video frames using sws_scale or using our own implementation, which is limited to type AV_PIX_FMT_YUV420P
