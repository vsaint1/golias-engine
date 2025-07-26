#pragma once

#include "core/engine.h"

constexpr int NUM_CHANNELS = 2;
constexpr int SAMPLE_RATE  = 48000;


/*!
    @brief Audio struct
    - mini-audio backend

    @version 0.0.9
*/
class Audio {

public:
    float volume   = 1.0f;
    float duration = 0.0f;

    /*!

    @brief Load Audio and keep track of it

    @param file_Path Path to the audio file

    @version 0.0.3


     * * @return Audio allocated audio memory struct
    */
    static Audio* load(const std::string& file_Path);


    /*!

    @brief Change the audio volume

    @param vol Audio volume from 0.0 to 1.0

    @version 0.0.3
 @return

     * * void
    */
    void set_volume(float vol);

    /*!

        @brief Pause audio and fade out

        @version 0.0.3
        @return void
    */
    void pause();

    /*!

    @brief Play audio

    @param loop Play audio in `loop` or not

    @version 0.0.3
    @return void
    */
    void play(bool loop = false);

    bool is_playing();

    void set_loop(bool loop);


    /*!

    @brief Unload Audio allocated memory
    - Manually clear allocated audio or at the end will be freed

     * * automatically

    @see CloseAudio


    @version 0.0.3
    @return void
    */
    void destroy();

    Audio(const Audio&)      = delete;
    Audio& operator=(Audio&) = delete;
    Audio(Audio&&)           = delete;

private:
    ma_decoder* decoder = nullptr;
    ma_sound sound{};
};


/*!

    @brief Initialize Sound Engine
    - mini-audio backend

    @version 0.0.3
    @return bool
*/
bool init_audio_engine();

/*!

    @brief Close Sound Engine and free allocated resources


    @version 0.0.3
    @return void
*/
void close_audio_engine();

/*!

    @brief Change the [audio engine] global volume

    @param volume Audio volume from [0.0f -> 1.f]

 @version

 * * 0.0.3
    @return void
*/
void Audio_SetMasterVolume(float volume);


// extern ma_engine* engine;
//
// extern Core core;

// TODO: create a Resource Manager
inline HashMap<std::string, Audio*> audios;
