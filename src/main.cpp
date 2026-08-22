#include <curl/curl.h>
#include <curl/easy.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/types.h>

#include <cjson/cJSON.h>

#include <audio.hpp>
#include <musicbrainz.hpp>

int main(int argc, const char *argv[]) {

    // CD-IO Setup
    CdIo_t *p_cdio = cdio_open("/dev/cdrom", DRIVER_UNKNOWN);
    if (!p_cdio) {
        std::cerr << "Failure opening CD-IO object";
        cdio_destroy(p_cdio);
        return 1;
    }

    // Get LSN (Logical Sector Number) of first two tracks
    // Use this information to fetch audio data of first track
    lsn_t t1_lsn = cdio_get_track_lsn(p_cdio, cdio_get_first_track_num(p_cdio));
    lsn_t t2_lsn = cdio_get_track_lsn(p_cdio, 2);

    std::cout << "t1 LSN: " << (int)t1_lsn << ", t2 LSN: " << (int)t2_lsn
              << "\n";
    long sectors_to_read = t2_lsn - t1_lsn;
    std::cout << "Sectors to read: " << sectors_to_read << "\n";

    // GET music brainz metadata for disc
    std::string mb_meta = get_mb_meta(p_cdio);

    // Parse JSON metadata response
    cJSON *mb_meta_json =
        cJSON_ParseWithLength(mb_meta.c_str(), mb_meta.length());
    cJSON *releaseJSON =
        cJSON_GetArrayItem(cJSON_GetObjectItem(mb_meta_json, "releases"), 0);
    char *mbid_raw = cJSON_Print(cJSON_GetObjectItem(releaseJSON, "id"));
    std::string mbid = std::string(mbid_raw);
    mbid = mbid.substr(1, mbid.size() - 2); // Remove quotes added by cJSON

    std::cout << "Release ID: " << mbid << "\n";

    std::string cover_art_response = get_cover_art_url(mbid);
    std::string cover_art_url = parse_cover_art_response(cover_art_response);
    std::string cover_art_img = get_cover_art_data(cover_art_url);

    // Temp - write cover art to file
    std::ofstream outFile("coverart.jpg");
    outFile << cover_art_img;

    // std::vector<uint8_t> audio_buffer;

    // read_sound(p_cdio, audio_buffer, t1_lsn, t2_lsn);

    // std::cout << "Audio buffer size in main after read_sound: "
    //           << audio_buffer.size() << std::endl;

    // play_audio(audio_buffer);

    std::cout << '\n' << std::endl;

    cdio_destroy(p_cdio);
    return 0;
}
