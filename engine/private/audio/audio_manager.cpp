#include "audio/audio_manager.h"
#include "audio/audio_buffer.h"

#include <spdlog/spdlog.h>

#define AUDIO_DEVICE_FORMAT    ma_format_f32    // Device output format (float-32bit)
#define AUDIO_DEVICE_CHANNELS  2                // Device output channels: stereo
#define AUDIO_DEVICE_SAMPLE_RATE 0              // Device output sample rate (0 = default)

namespace golias {

    static void OnSendAudioDataToDevice(ma_device* pDevice, void* pFramesOut, const void* pFramesInput, ma_uint32 frameCount);
    static void MixAudioFrames(float* framesOut, const float* framesIn, ma_uint32 frameCount, rAudioBuffer* buffer);

    AudioManager::AudioManager() {
        maDevice = std::make_unique<ma_device>();
        maContext = std::make_unique<ma_context>();
        maMutex = new ma_mutex;
    }

    AudioManager::~AudioManager() {
        if (isReady) {
            ma_mutex_uninit(static_cast<ma_mutex*>(maMutex));
            free(maMutex);
            ma_device_uninit(maDevice.get());
            ma_context_uninit(maContext.get());
            isReady = false;
        } else if (maMutex) {
            free(maMutex);
        }
    }

    bool AudioManager::Initialize() {
        ma_context_config ctxConfig = ma_context_config_init();
        
        ma_result result = ma_context_init(nullptr, 0, &ctxConfig, maContext.get());
        if (result != MA_SUCCESS) {
            spdlog::error("AudioManager::Initialize: Failed to initialize context");
            return false;
        }

        
        ma_device_config config = ma_device_config_init(ma_device_type_playback);
        config.playback.pDeviceID = nullptr;  // NULL for default device
        config.playback.format = AUDIO_DEVICE_FORMAT;
        config.playback.channels = AUDIO_DEVICE_CHANNELS;
        config.sampleRate = AUDIO_DEVICE_SAMPLE_RATE;
        config.dataCallback = OnSendAudioDataToDevice;
        config.pUserData = this;  // Pass AudioManager instance to callback

        result = ma_device_init(maContext.get(), &config, maDevice.get());
        if (result != MA_SUCCESS) {
            spdlog::error("AudioManager::Initialize: Failed to initialize playback device");
            ma_context_uninit(maContext.get());
            return false;
        }

        if (ma_mutex_init(static_cast<ma_mutex*>(maMutex)) != MA_SUCCESS) {
            spdlog::error("AudioManager::Initialize: Failed to create mutex for mixing");
            ma_device_uninit(maDevice.get());
            ma_context_uninit(maContext.get());
            return false;
        }

        result = ma_device_start(maDevice.get());
        if (result != MA_SUCCESS) {
            spdlog::error("AudioManager::Initialize: Failed to start playback device");
            ma_mutex_uninit(static_cast<ma_mutex*>(maMutex));
            ma_device_uninit(maDevice.get());
            ma_context_uninit(maContext.get());
            return false;
        }

        isReady = true;
        spdlog::info("AudioManager::Initialize: Device initialized successfully");
        spdlog::info("    > Backend:       {}", ma_get_backend_name(maContext->backend));
        spdlog::info("    > Format:        {} -> {}", ma_get_format_name(maDevice->playback.format), ma_get_format_name(maDevice->playback.internalFormat));
        spdlog::info("    > Channels:      {} -> {}", maDevice->playback.channels, maDevice->playback.internalChannels);
        spdlog::info("    > Sample rate:   {} -> {}", maDevice->sampleRate, maDevice->playback.internalSampleRate);

        return true;
    }

    ma_device* AudioManager::GetDevice() {
        return maDevice.get();
    }

    ma_context* AudioManager::GetContext() {
        return maContext.get();
    }

    void* AudioManager::GetLock() {
        return maMutex;
    }

    void AudioManager::SetListenerPosition(float x, float y, float z) {
        listenerPosX = x;
        listenerPosY = y;
        listenerPosZ = z;
    }

    void AudioManager::SetMasterVolume(float volume) {
        masterVolume = volume;
        if (isReady) {
            ma_device_set_master_volume(maDevice.get(), masterVolume);
        }
    }

    float AudioManager::GetMasterVolume() const {
        return masterVolume;
    }

    void AudioManager::TrackAudioBuffer(rAudioBuffer* buffer) {
        ma_mutex_lock(static_cast<ma_mutex*>(maMutex));
        {
            if (firstBuffer == nullptr) {
                firstBuffer = buffer;
                lastBuffer = buffer;
            } else {
                lastBuffer->next = buffer;
                buffer->prev = lastBuffer;
                lastBuffer = buffer;
            }
        }
        ma_mutex_unlock(static_cast<ma_mutex*>(maMutex));
    }

    void AudioManager::UntrackAudioBuffer(rAudioBuffer* buffer) {
        ma_mutex_lock(static_cast<ma_mutex*>(maMutex));
        {
            if (buffer->prev == nullptr) {
                firstBuffer = buffer->next;
            } else {
                buffer->prev->next = buffer->next;
            }

            if (buffer->next == nullptr) {
                lastBuffer = buffer->prev;
            } else {
                buffer->next->prev = buffer->prev;
            }

            buffer->prev = nullptr;
            buffer->next = nullptr;
        }
        ma_mutex_unlock(static_cast<ma_mutex*>(maMutex));
    }

    static ma_uint32 ReadAudioBufferFramesInInternalFormat(rAudioBuffer* audioBuffer, void* framesOut, ma_uint32 frameCount) {
        ma_uint32 subBufferSizeInFrames = (audioBuffer->sizeInFrames > 1) ? audioBuffer->sizeInFrames / 2 : audioBuffer->sizeInFrames;
        ma_uint32 currentSubBufferIndex = audioBuffer->frameCursorPos / subBufferSizeInFrames;

        if (currentSubBufferIndex > 1) return 0;

        ma_uint32 framesRemainingInOutputBuffer = frameCount;
        ma_uint32 framesRead = 0;

        while (framesRemainingInOutputBuffer > 0 && (audioBuffer->frameCursorPos < audioBuffer->sizeInFrames)) {
            ma_uint32 framesRemainingInSubBuffer = subBufferSizeInFrames - (audioBuffer->frameCursorPos % subBufferSizeInFrames);
            ma_uint32 framesToRead = framesRemainingInOutputBuffer;
            if (framesToRead > framesRemainingInSubBuffer) framesToRead = framesRemainingInSubBuffer;

            if (framesOut != nullptr) {
                ma_uint32 bytesPerFrame = audioBuffer->channels * ma_get_bytes_per_sample(audioBuffer->format);
                memcpy((unsigned char*)framesOut + (framesRead * bytesPerFrame),
                    (unsigned char*)audioBuffer->data + (audioBuffer->frameCursorPos * bytesPerFrame),
                    framesToRead * bytesPerFrame);
            }

            audioBuffer->frameCursorPos = (audioBuffer->frameCursorPos + framesToRead) % audioBuffer->sizeInFrames;
            framesRemainingInOutputBuffer -= framesToRead;
            framesRead += framesToRead;

            if ((framesToRead == framesRemainingInSubBuffer) && !audioBuffer->looping) {
                audioBuffer->isSubBufferProcessed[currentSubBufferIndex] = true;
                if (audioBuffer->isSubBufferProcessed[0] && audioBuffer->isSubBufferProcessed[1]) {
                    audioBuffer->playing = false;
                    break;
                }
            }

            if (!audioBuffer->looping && (audioBuffer->frameCursorPos == 0)) {
                audioBuffer->playing = false;
                break;
            }
        }

        return framesRead;
    }

    static ma_uint32 ReadAudioBufferFramesInMixingFormat(rAudioBuffer* audioBuffer, float* framesOut, ma_uint32 frameCount) {
        ma_uint8 inputBuffer[4096] = { 0 };
        ma_uint32 totalOutputFramesProcessed = 0;
        ma_uint32 outputChannels = AUDIO_DEVICE_CHANNELS;

        while (totalOutputFramesProcessed < frameCount) {
            ma_uint32 outputFramesToProcessThisIteration = frameCount - totalOutputFramesProcessed;
            ma_uint32 maxFrames = sizeof(inputBuffer) / ma_get_bytes_per_frame(audioBuffer->format, audioBuffer->channels);
            if (outputFramesToProcessThisIteration > maxFrames) {
                outputFramesToProcessThisIteration = maxFrames;
            }

            float* runningFramesOut = framesOut + (totalOutputFramesProcessed * outputChannels);
            
            ma_uint64 inputFramesToProcess64 = outputFramesToProcessThisIteration;
            ma_uint64 outputFrames64 = outputFramesToProcessThisIteration;
            ma_data_converter_get_required_input_frame_count(&audioBuffer->converter, outputFrames64, &inputFramesToProcess64);
            ma_uint32 inputFramesToProcess = (ma_uint32)inputFramesToProcess64;
            
            if (inputFramesToProcess > audioBuffer->sizeInFrames) inputFramesToProcess = audioBuffer->sizeInFrames;

            ma_uint32 inputFramesProcessed = ReadAudioBufferFramesInInternalFormat(audioBuffer, inputBuffer, inputFramesToProcess);

            ma_uint64 inputFramesProcessedThisIteration = inputFramesProcessed;
            ma_uint64 outputFramesProcessedThisIteration = outputFramesToProcessThisIteration;
            ma_data_converter_process_pcm_frames(&audioBuffer->converter, inputBuffer, &inputFramesProcessedThisIteration, runningFramesOut, &outputFramesProcessedThisIteration);

            totalOutputFramesProcessed += (ma_uint32)outputFramesProcessedThisIteration;

            if (inputFramesProcessedThisIteration < inputFramesToProcess) break;
            if (inputFramesProcessedThisIteration == 0 && outputFramesProcessedThisIteration == 0) break;
        }

        return totalOutputFramesProcessed;
    }

    static void OnSendAudioDataToDevice(ma_device* pDevice, void* pFramesOut, const void* pFramesInput, ma_uint32 frameCount) {
        AudioManager* manager = static_cast<AudioManager*>(pDevice->pUserData);

        memset(pFramesOut, 0, frameCount * pDevice->playback.channels * ma_get_bytes_per_sample(pDevice->playback.format));

        ma_mutex_lock(static_cast<ma_mutex*>(manager->GetLock()));
        {
            for (rAudioBuffer* audioBuffer = manager->GetFirstBuffer(); audioBuffer != nullptr; audioBuffer = audioBuffer->next) {
                
                if (!audioBuffer->playing || audioBuffer->paused) continue;

                ma_uint32 framesRead = 0;

                while (framesRead < frameCount) {
                    float tempBuffer[1024];
                    ma_uint32 framesToReadRightNow = frameCount - framesRead;
                    if (framesToReadRightNow > sizeof(tempBuffer) / sizeof(tempBuffer[0]) / AUDIO_DEVICE_CHANNELS) {
                        framesToReadRightNow = sizeof(tempBuffer) / sizeof(tempBuffer[0]) / AUDIO_DEVICE_CHANNELS;
                    }

                    ma_uint32 framesJustRead = ReadAudioBufferFramesInMixingFormat(audioBuffer, tempBuffer, framesToReadRightNow);
                    if (framesJustRead > 0) {
                        float* framesOut = (float*)pFramesOut + (framesRead * pDevice->playback.channels);
                        MixAudioFrames(framesOut, tempBuffer, framesJustRead, audioBuffer);
                        framesRead += framesJustRead;
                    }

                    if (framesJustRead < framesToReadRightNow) {
                        if (!audioBuffer->looping) {
                            audioBuffer->playing = false;
                            break;
                        }
                    }
                }
            }
        }
        ma_mutex_unlock(static_cast<ma_mutex*>(manager->GetLock()));
    }

    static void MixAudioFrames(float* framesOut, const float* framesIn, ma_uint32 frameCount, rAudioBuffer* buffer) {
        const float localVolume = buffer->volume;
        const ma_uint32 channels = AUDIO_DEVICE_CHANNELS;

        if (channels == 2) {  // Stereo with panning
            const float left = buffer->pan;
            const float right = 1.0f - left;

            // Fast sine approximation for pan law
            const float leftVolume = localVolume * 0.5f * left * (3.0f - left * left);
            const float rightVolume = localVolume * 0.5f * right * (3.0f - right * right);

            for (ma_uint32 frame = 0; frame < frameCount; frame++) {
                framesOut[frame * 2 + 0] += framesIn[frame * 2 + 0] * leftVolume;
                framesOut[frame * 2 + 1] += framesIn[frame * 2 + 1] * rightVolume;
            }
        } else {  // Mono or multi-channel (no panning)
            for (ma_uint32 frame = 0; frame < frameCount; frame++) {
                for (ma_uint32 channel = 0; channel < channels; channel++) {
                    framesOut[frame * channels + channel] += framesIn[frame * channels + channel] * localVolume;
                }
            }
        }
    }

} // namespace golias
