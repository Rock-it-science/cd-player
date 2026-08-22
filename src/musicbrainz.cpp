#include <curl/curl.h>
#include <curl/easy.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/types.h>

#include <cjson/cJSON.h>

#include <cdio/cdio.h>
#include <cdio/cdtext.h>
#include <cdio/device.h>
#include <cdio/disc.h>
#include <cdio/read.h>
#include <cdio/sector.h>
#include <cdio/track.h>
#include <cdio/types.h>

const char *MB_USER_AGENT = "MyCdPlayer/0.0.1 (wilmc17@gmail.com)";

/*
    MusicBrainz Table of Contents (TOC) for disc fuzzy-matching lookups
    The TOC consists of the following:

        First track (always 1)
        total number of tracks
        sector offset of the leadout (end of the disc)
        a list of sector offsets for each track, beginning with track 1
   (generally 150 sectors)
 */
std::string form_mb_toc(CdIo_t *p_cdio) {
    track_t num_tracks = cdio_get_num_tracks(p_cdio);
    std::string num_tracks_s = std::to_string(num_tracks);
    std::string leadout_lsn_s =
        std::to_string(cdio_get_track_lsn(p_cdio, CDIO_CDROM_LEADOUT_TRACK));

    std::string sector_offsets_concat;
    for (int i = 1; i <= num_tracks; i++) {
        sector_offsets_concat +=
            std::to_string(cdio_get_track_lsn(p_cdio, i)) + "+";
    };
    sector_offsets_concat.pop_back();

    std::string out_string =
        "1+" + num_tracks_s + "+" + leadout_lsn_s + "+" + sector_offsets_concat;
    return out_string;
}

struct memory {
    char *response;
    size_t size;
};

/*
    Callback function for CURLOPT_WRITEFUNCTION to write the CURL response to
   memory.
   data contains the payload.
   nmemb is the size of the data payload.
   size is always 1.
 */
static size_t curl_write_to_buffer(char *data, size_t size, size_t nmemb,
                                   void *clientp) {

    ((std::string *)clientp)->append((char *)data, nmemb);
    return nmemb;
}

/*
    Use CD-IO object to form query that can be used to get metadata from
   MusicBrainz in a curl request
 */
std::string get_mb_meta(CdIo_t *p_cdio) {
    std::string mb_toc = form_mb_toc(p_cdio);
    std::cout << mb_toc << "\n";

    // Fetch URL for musicbrainz API
    //      Don't have disc ID, so use table of contents that we parsed from
    //      track count and lengths
    //      Limit to 1 release since we only need to get the disc ID of the
    //      master release for the metadata we need
    //      Use fmt=json to return as JSON instead of XML
    // TODO - there might be a better way to find the "main" release than just
    // limit=1 here
    std::string mb_fetch_url =
        "https://musicbrainz.org/ws/2/discid/-?toc=" + mb_toc +
        "&limit=1&fmt=json";

    CURL *curl = curl_easy_init();
    std::string read_buffer;

    curl_easy_setopt(curl, CURLOPT_URL, mb_fetch_url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, MB_USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_to_buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        std::cerr << "CURL error code: " << result << "\n";
    }

    // Cleanup
    curl_easy_cleanup(curl);

    return read_buffer;
}

/*
    Get cover art URL from cover art archive by looking up music brainz release
   ID
 */
std::string get_cover_art_url(std::string release_id) {

    std::string cover_art_fetch_url =
        "https://coverartarchive.org/release/" + release_id;

    std::cout << "Fetching cover art url from: " << cover_art_fetch_url << "\n";

    CURL *curl = curl_easy_init();
    std::string read_buffer;

    curl_easy_setopt(curl, CURLOPT_URL, cover_art_fetch_url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, MB_USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_to_buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        std::cerr << "CURL error code: " << result << "\n";
    }
    curl_easy_cleanup(curl);

    return read_buffer;
}

/*
    Parse response from cover art archive to get image url
 */
std::string parse_cover_art_response(std::string cover_art_response) {
    /*
        Example response:
        {
          "images":[
             {
                "types":[
                   "Front"
                ],
                "front":true,
                "back":false,
                "edit":17462565,
                "image":"http://coverartarchive.org/release/76df3287-6cda-33eb-8e9a-044b5e15ffdd/829521842.jpg",
                "comment":"",
                "approved":true,
                "id":"829521842",
                "thumbnails":{
                  "250":"http://coverartarchive.org/release/76df3287-6cda-33eb-8e9a-044b5e15ffdd/829521842-250.jpg",
                  "500":"http://coverartarchive.org/release/76df3287-6cda-33eb-8e9a-044b5e15ffdd/829521842-500.jpg",
                  "1200":"http://coverartarchive.org/release/76df3287-6cda-33eb-8e9a-044b5e15ffdd/829521842-1200.jpg",
                  "small":"http://coverartarchive.org/release/76df3287-6cda-33eb-8e9a-044b5e15ffdd/829521842-250.jpg",
                  "large":"http://coverartarchive.org/release/76df3287-6cda-33eb-8e9a-044b5e15ffdd/829521842-500.jpg"
                }
             }
          ],
          "release":"http://musicbrainz.org/release/76df3287-6cda-33eb-8e9a-044b5e15ffdd"
        }
     */
    cJSON *cover_art_json = cJSON_ParseWithLength(cover_art_response.c_str(),
                                                  cover_art_response.length());
    cJSON *cover_art_url_json = cJSON_GetObjectItem(
        cJSON_GetArrayItem(cJSON_GetObjectItem(cover_art_json, "images"), 0),
        "image");
    std::string image_url = cJSON_Print(cover_art_url_json);
    image_url = image_url.substr(2, image_url.length() - 2);

    return image_url;
}

/*
    Given direct URL of cover art, fetch the jpg data bytes
 */
std::string get_cover_art_data(std::string cover_art_url) {
    CURL *curl = curl_easy_init();
    std::string read_buffer;

    std::cout << "Fetching cover art data from: " << cover_art_url << "\n";

    curl_easy_setopt(curl, CURLOPT_URL, cover_art_url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, MB_USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_to_buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        std::cerr << "CURL error code: " << result << "\n";
    }
    curl_easy_cleanup(curl);

    return read_buffer;
}
