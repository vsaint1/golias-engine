#pragma once

#include <miniaudio.h>

// Audio buffer struct - internal structure for audio mixing
struct rAudioBuffer {
    ma_data_converter converter;    // Audio data converter

    float volume = 1.0f;            // Audio buffer volume
    float pitch = 1.0f;             // Audio buffer pitch
    float pan = 0.5f;               // Audio buffer pan (0.0f to 1.0f)

    bool playing = false;           // Audio buffer state: playing
    bool paused = false;            // Audio buffer state: paused
    bool looping = false;           // Audio buffer looping
    int usage = 0;                  // Audio buffer usage mode: STATIC(0) or STREAM(1)

    bool isSubBufferProcessed[2] = { true, true };  // SubBuffer processed (virtual double buffer)
    unsigned int sizeInFrames = 0;                  // Total buffer size in frames
    unsigned int frameCursorPos = 0;                // Frame cursor position
    unsigned int framesProcessed = 0;               // Total frames processed in this buffer (required for play timing)

    unsigned char* data = nullptr;  // Data buffer, on music stream keeps filling
    
    // Format information (stored to avoid accessing internal converter fields)
    ma_format format = ma_format_f32;
    ma_uint32 channels = 2;
    ma_uint32 sampleRate = 0;

    // 3D audio spatialization
    ma_spatializer* spatializer = nullptr;
    bool spatializationEnabled = false;
    float positionX = 0.0f;
    float positionY = 0.0f;
    float positionZ = 0.0f;

    rAudioBuffer* next = nullptr;   // Next audio buffer on the list
    rAudioBuffer* prev = nullptr;   // Previous audio buffer on the list
};

enum rAudioBufferUsage {
    AUDIO_BUFFER_USAGE_STATIC = 0,
    AUDIO_BUFFER_USAGE_STREAM = 1
};
