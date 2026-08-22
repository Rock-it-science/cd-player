#include <cdio/cdio.h>
#include <string>

/*
    MusicBrainz Table of Contents (TOC) for disc fuzzy-matching lookups
    The TOC consists of the following:

        First track (always 1)
        total number of tracks
        sector offset of the leadout (end of the disc)
        a list of sector offsets for each track, beginning with track 1
   (generally 150 sectors)
 */
std::string form_mb_toc(CdIo_t *p_cdio);

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
                                   void *clientp);

/*
    Use CD-IO object to form query that can be used to get metadata from
   MusicBrainz in a curl request
 */
std::string get_mb_meta(CdIo_t *p_cdio);

/*
    Get cover art URL from cover art archive by looking up music brainz release
   ID
 */
std::string get_cover_art_url(std::string release_id);
/*
    Parse response from cover art archive to get image url
 */
std::string parse_cover_art_response(std::string cover_art_response);

/*
    Given direct URL of cover art, fetch the jpg data bytes
 */
std::string get_cover_art_data(std::string cover_art_url);
