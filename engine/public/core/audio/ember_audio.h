#pragma once

#include "core/engine.h"

#define NUM_CHANNELS 2
#define SAMPLE_RATE 44100

struct Audio {
    ma_decoder* decoder;
    ma_sound sound;
    float volume = 1.0f;
    float duration = 0.0f;
};

/*!

    @brief Initialize Sound Engine
    - mini-audio backend

    @version 0.0.3
    @return bool
*/
bool InitAudio();

/*!

    @brief Close Sound Engine and free allocated resources


    @version 0.0.3
    @return void
*/
void CloseAudio();


/*!

    @brief Load Audio and keep track of it

    @param file_Path Path to the audio file

    @version 0.0.3
    @return Audio allocated audio memory struct
*/
Audio* Mix_LoadAudio(const std::string& file_Path);

/*!

    @brief Play audio

    @param music Valid Audio struct pointer
    @param loop Play audio in `loop` or not

    @version 0.0.3
    @return void
*/
void Mix_PlayAudio(Audio* audio, bool loop = false);


/*!

    @brief Pause audio and fade out

    @param audio Valid Audio struct pointer

    @version 0.0.3
    @return void
*/
void Mix_PauseAudio(Audio* audio);

/*!

    @brief Change the audio volume

    @param audio Valid Audio struct pointer
    @param volume Audio volume from 0.0 to 1.0

    @version 0.0.3
    @return void
*/
void Mix_SetVolume(Audio* audio, float volume);

/*!

    @brief Change the [audio engine] global volume

    @param volume Audio volume from 0 to 100

    @version 0.0.3
    @return void
*/
void Mix_SetGlobalVolume(float volume);


/*!

    @brief Unload Audio allocated memory
    - Manually clear allocated audio or at the end will be freed automatically

    @see CloseAudio

    @param audio Valid Audio struct pointer

    @version 0.0.3
    @return void
*/
void Mix_UnloadAudio(Audio* audio);


extern ma_engine engine;

extern Core core;

// TODO: create a Resource Manager
inline std::unordered_map<std::string, Audio*> audios;