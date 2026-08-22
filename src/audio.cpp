#include <cstdint>
#include <iostream>
#include <iterator>
#include <stdlib.h>
#include <vector>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <cdio/cdio.h>
#include <cdio/cdtext.h>
#include <cdio/device.h>
#include <cdio/disc.h>
#include <cdio/read.h>
#include <cdio/sector.h>
#include <cdio/track.h>
#include <cdio/types.h>

void read_sound(CdIo_t *p_cdio, std::vector<uint8_t> &audio_buffer,
                lsn_t lsn_start, lsn_t lsn_end) {

    std::cout << "Reading sectors from " << lsn_start << " to " << lsn_end
              << "...\n"
              << std::flush;
    uint8_t sector_buffer[CDIO_CD_FRAMESIZE_RAW];
    for (lsn_t lsn_i = lsn_start; lsn_i < lsn_end; lsn_i++) {
        driver_return_code_t read_audio_sectors_return =
            cdio_read_audio_sector(p_cdio, sector_buffer, lsn_i);
        if (read_audio_sectors_return != 0) { // Failure
            std::cerr << "Failure reading audio sector! Return code "
                      << read_audio_sectors_return << "\n";
            cdio_destroy(p_cdio);
            return;
        }

        // Load sector buffer to tip of audio_buffer one-byte-at-a-time
        int audio_buffer_bytes_tip = lsn_i * CDIO_CD_FRAMESIZE_RAW;
        audio_buffer.insert(audio_buffer.begin() + audio_buffer_bytes_tip,
                            std::begin(sector_buffer), std::end(sector_buffer));
    }
    std::cout << "Audio buffer size in read_sound: " << audio_buffer.size()
              << std::endl;
    return;
}

/* MiniAudio */

int play_audio(const std::vector<uint8_t> audio_buffer) {
    ma_result result;
    ma_engine engine;
    ma_sound sound;
    ma_audio_buffer ma_audio_buffer;

    std::cout << "Audio buffer size in play_audio: " << audio_buffer.size()
              << std::endl;

    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        std::cerr << "Failure initializing MA engine";
        return result;
    }

    // Create a MiniAudio buffer from the buffer we read from the CD
    int sizeInFrames =
        audio_buffer.size() / 4; // 1 Frame = 2 bytes * 2 Channels
    ma_audio_buffer_config buffer_config = ma_audio_buffer_config_init(
        ma_format_s16, 2, sizeInFrames, audio_buffer.data(), NULL);
    buffer_config.sampleRate =
        44100; // Standard CD Sample Rate (default is device's default of 48000)
    ma_audio_buffer_init(&buffer_config, &ma_audio_buffer);

    // Create sound from data source
    ma_sound_init_from_data_source(&engine, &ma_audio_buffer, 0, NULL, &sound);
    std::cout << "Playing sound\n";
    result = ma_sound_start(&sound);
    if (result != MA_SUCCESS) {
        std::cerr << "Failure playing sound, code: " << result;
        return result;
    }

    // Keep audio running until user stops it
    printf("Press Enter to quit...");
    getchar();

    ma_engine_uninit(&engine);
    return 0;
}
