
#include <cdio/cdio.h>
#include <cstdint>
#include <miniaudio.h>
#include <stdlib.h>
#include <vector>

/*
    Read sound sectors between given LSNs into buffer
 */
void read_sound(CdIo_t *p_cdio, std::vector<uint8_t> &audio_buffer,
                lsn_t lsn_start, lsn_t lsn_end);

/*
    Given buffer of sound data, play sound
 */
int play_audio(const std::vector<uint8_t> audio_buffer);
