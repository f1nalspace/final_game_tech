#ifndef MINIMP3_EXT_H
#define MINIMP3_EXT_H
/*
    https://github.com/lieff/minimp3
    To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide.
    This software is distributed without any warranty.
    See <http://creativecommons.org/publicdomain/zero/1.0/>.
*/
#include "minimp3.h"

#define MP3D_SEEK_TO_BYTE   0
#define MP3D_SEEK_TO_SAMPLE 1
#define MP3D_SEEK_TO_SAMPLE_INDEXED 2

typedef struct
{
    mp3d_sample_t *buffer;
    size_t samples; /* channels included, byte size = samples*sizeof(int16_t) */
    int channels, hz, layer, avg_bitrate_kbps;
} mp3dec_file_info_t;

typedef struct
{
    const uint8_t *buffer;
    size_t size;
} mp3dec_map_info_t;

typedef struct
{
    mp3dec_t mp3d;
    mp3dec_map_info_t file;
    int seek_method;
#ifndef MINIMP3_NO_STDIO
    int is_file;
#endif
} mp3dec_ex_t;

typedef int (*MP3D_ITERATE_CB)(void *user_data, const uint8_t *frame, int frame_size, size_t offset, mp3dec_frame_info_t *info);
typedef int (*MP3D_PROGRESS_CB)(void *user_data, size_t file_size, size_t offset, mp3dec_frame_info_t *info);

#ifdef __cplusplus
extern "C" {
#endif

/* decode whole buffer block */
void mp3dec_load_buf(mp3dec_t *dec, const uint8_t *buf, size_t buf_size, mp3dec_file_info_t *info, MP3D_PROGRESS_CB progress_cb, void *user_data);
/* iterate through frames with optional decoding */
void mp3dec_iterate_buf(const uint8_t *buf, size_t buf_size, MP3D_ITERATE_CB callback, void *user_data);
/* decoder with seeking capability */
int mp3dec_ex_open_buf(mp3dec_ex_t *dec, const uint8_t *buf, size_t buf_size, int seek_method);
void mp3dec_ex_close(mp3dec_ex_t *dec);
void mp3dec_ex_seek(mp3dec_ex_t *dec, size_t position);
int mp3dec_ex_read(mp3dec_ex_t *dec, int16_t *buf, int samples);
#ifndef MINIMP3_NO_STDIO
/* stdio versions with file pre-load */
int mp3dec_load(mp3dec_t *dec, const char *file_name, mp3dec_file_info_t *info, MP3D_PROGRESS_CB progress_cb, void *user_data);
int mp3dec_iterate(const char *file_name, MP3D_ITERATE_CB callback, void *user_data);
int mp3dec_ex_open(mp3dec_ex_t *dec, const char *file_name, int seek_method);
#endif

#ifdef __cplusplus
}
#endif
#endif /*MINIMP3_EXT_H*/

#ifdef MINIMP3_IMPLEMENTATION

typedef enum {
    mp3dec_endianess_type_little = 0x04030201,
    mp3dec_endianess_type_big = 0x01020304,
} mp3dec_endianess_type;

typedef union {
	unsigned char bytes[4]; 
	uint32_t value; 
} mp3dec_endianess;

static const mp3dec_endianess mp3dec__global_endianessOrder = { 1, 2, 3, 4 };

static bool is_mp3dec_big_endian() {
    return mp3dec__global_endianessOrder.value == mp3dec_endianess_type_big;
}

#define is_mp3dec_bitset(value, bit) (((value) >> (bit)) & 0x1)

// ID3v2 Tag
// Version 2
// Version 3
// Version 4

#pragma pack(push, 1)
typedef struct {
    char id[3];       // ID3 identifier ("ID3")
    uint8_t version;  // Version (major version in the high nibble, revision in the low nibble)
    uint8_t revision; // Revision number
    uint8_t flags;    // Flags
    uint32_t size;    // Synchsafe-Size of the tag (excluding the header)
} mp3dec_id3v2_header;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    char id[4];      // 4-character frame ID (e.g., "TIT2" for title)
    uint32_t size;   // Synchsafe-Size of the tag (excluding the header)
    uint16_t flags;  // Frame flags (2 bytes)
} mp3dec_id3v2_frame_header;
#pragma pack(pop)

static uint32_t mp3dec_u32_reverse(const uint32_t x) {
    return ((x >> 24) & 0x000000FF) | ((x >> 8) & 0x0000FF00) | ((x << 8) & 0x00FF0000) | ((x << 24) & 0xFF000000);
}

static uint32_t mp3dec_u32_bigendian_to_native(const uint32_t u32be) {
    if (is_mp3dec_big_endian())
        return u32be;
    else
        return mp3dec_u32_reverse(u32be);
}

static uint32_t mp3dec_id3v2_get_syncsafe_size(const uint32_t syncsafe) {
    uint32_t a = (syncsafe >> 0) & 0xFF;
    uint32_t b = (syncsafe >> 8) & 0xFF;
    uint32_t c = (syncsafe >> 16) & 0xFF;
    uint32_t d = (syncsafe >> 24) & 0xFF;
    uint32_t result = (a << 0) | (b << 7) | (c << 14) | (d << 21);
    return result;
}

const char *mp3dec_g_supportedID3v2FrameHeaderIDs[] = {
    "AENC", // Audio encryption
    "APIC", // Attached picture
    "COMM", // Comments
    "COMR", // Commercial frame
    "ENCR", // Encryption method registration
    "EQUA", // Equalization
    "ETCO", // Event timing codes
    "GEOB", // General encapsulated object
    "GRID", // Group identification registration
    "IPLS", // Involved people list
    "LINK", // Linked information
    "MCDI", // Music CD identifier
    "MIDI", // MIDI
    "MLLT", // MPEG location lookup table
    "OWNE", // Ownership frame
    "PRIV", // Private frame
    "PCNT", // Play counter
    "POPM", // Popularimeter
    "POSS", // Position synchronization frame
    "RBUF", // Recommended buffer size
    "RVAD", // Relative volume adjustment
    "RVRB", // Reverb
    "SEEK", // Seek frame
    "SIGN", // Signature frame
    "SYLT", // Synchronized lyrics/text
    "SYTC", // Synchronized tempo codes
    "TALB", // Album/Movie/Show title
    "TPE1", // Lead performer(s)/Soloist(s)
    "TPE2", // Band/orchestra/accompaniment
    "TPE3", // Conductor/performer refinement
    "TPE4", // Interpreted, remixed, or otherwise modified by
    "TIT1", // Content group description
    "TIT2", // Track title
    "TIT3", // Subtitle/Description refinement
    "TCON", // Content type (genre)
    "TPUB", // Publisher
    "TCOM", // Composer
    "TRCK", // Track number/Position in set
    "TYER", // Year
    "TSSE", // Software/Hardware and settings used for encoding
    "TXXX", // User-defined text information frame
    "USLT", // Unsynchronized lyric/text transcription
    "WCOM", // Commercial information
    "WCOP", // Copyright/Legal information
    "WOAF", // Official audio file webpage
    "WOAR", // Official artist/performer webpage
    "WOAS", // Official audio source webpage
    "WORS", // Official radio station webpage
    "WPAY", // Payment URL
    "WPUB", // Publisher's official webpage
};

#define MP3DEC_ID3V2_TAG_ID_COUNT sizeof(mp3dec_g_supportedID3v2FrameHeaderIDs) / sizeof(mp3dec_g_supportedID3v2FrameHeaderIDs[0])

static bool mp3dec_is_id3v2_frame_header_id(const char *id) {
    for (int i = 0; i < MP3DEC_ID3V2_TAG_ID_COUNT; ++i) {
        const char *t = mp3dec_g_supportedID3v2FrameHeaderIDs[i];
        if (strncmp(t, id, 4) == 0) {
            return true;
        }
    }
    return false;
}

static size_t mp3dec_skip_id3v2(const uint8_t *buf, size_t buf_size)
{
    if (buf_size <= 10 || memcmp(buf, "ID3", 3) != 0) {
        return 0;
    }

    const uint8_t *start = buf;

    size_t headerSize = sizeof(mp3dec_id3v2_header);
    assert(headerSize == 10);

    const mp3dec_id3v2_header *header = (const mp3dec_id3v2_header *)buf;
    buf_size -= headerSize;
    buf += headerSize;

    uint32_t tagSize = mp3dec_u32_bigendian_to_native(header->size);
    tagSize = mp3dec_id3v2_get_syncsafe_size(tagSize);

    // Skip extended header, if present
    if (is_mp3dec_bitset(header->flags, 6) && buf_size >= 4) {
        uint32 *x = (uint32 *)buf;
        uint32_t extendedHeaderSize = mp3dec_u32_bigendian_to_native(*x);
        extendedHeaderSize = mp3dec_id3v2_get_syncsafe_size(extendedHeaderSize);
        buf_size -= 4;
        buf += 4;
        if (buf_size >= extendedHeaderSize) {
            buf_size -= extendedHeaderSize;
            buf += extendedHeaderSize;
        }
    }

    size_t minSize = tagSize + headerSize;

    size_t frameHeaderSize = sizeof(mp3dec_id3v2_frame_header);
    assert(frameHeaderSize == 10);

    while (buf_size >= sizeof(mp3dec_id3v2_frame_header)) {
        const mp3dec_id3v2_frame_header *frameHeader = (const mp3dec_id3v2_frame_header *)buf;
        if (memcmp(frameHeader->id, "\0\0\0\0", 4) == 0) {
            // Done, we are inside the padding
            break;
        }
        if (!mp3dec_is_id3v2_frame_header_id(frameHeader->id)) {
            // Done, not a frame header
            break;
        }
        buf_size -= sizeof(*frameHeader);
        buf += sizeof(*frameHeader);

        uint32_t frameTagSize = mp3dec_u32_bigendian_to_native(frameHeader->size);
        if (header->version == 4) {
            frameTagSize = mp3dec_id3v2_get_syncsafe_size(frameTagSize);
        }

        if (buf_size >= frameTagSize) {
            buf_size -= frameTagSize;
            buf += frameTagSize;
        } else {
            break;
        }
    }

    size_t parsedSize = (size_t)(buf - start);

    size_t maxSize = minSize > parsedSize ? minSize : parsedSize;

    return maxSize;
}

void mp3dec_load_buf(mp3dec_t *dec, const uint8_t *buf, size_t buf_size, mp3dec_file_info_t *info, MP3D_PROGRESS_CB progress_cb, void *user_data)
{
    size_t orig_buf_size = buf_size;
    mp3d_sample_t pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
    mp3dec_frame_info_t frame_info;
    memset(info, 0, sizeof(*info));
    memset(&frame_info, 0, sizeof(frame_info));
    /* skip id3v2 */
    size_t id3v2size = mp3dec_skip_id3v2(buf, buf_size);
    if (id3v2size > buf_size)
        return;
    buf      += id3v2size;
    buf_size -= id3v2size;
#ifdef MINIMP3_SKIP_ID3V1
    if (buf_size > 128 && !strncmp((char *)buf + buf_size - 128, "TAG", 3))
    {
        buf_size -= 128;
        if (buf_size > 227 && !strncmp((char *)buf + buf_size - 227, "TAG+", 4))
            buf_size -= 227;
    }
#endif
    /* try to make allocation size assumption by first frame */
    mp3dec_init(dec);
    int samples;
    do
    {
        samples = mp3dec_decode_frame(dec, buf, (int)buf_size, pcm, &frame_info);
        buf      += frame_info.frame_bytes;
        buf_size -= frame_info.frame_bytes;
        if (samples)
            break;
    } while (frame_info.frame_bytes);
    if (!samples)
        return;
    samples *= frame_info.channels;
    size_t allocated = (buf_size/frame_info.frame_bytes)*samples*sizeof(mp3d_sample_t) + MINIMP3_MAX_SAMPLES_PER_FRAME*sizeof(mp3d_sample_t);
    info->buffer = (mp3d_sample_t*)malloc(allocated);
    if (!info->buffer)
        return;
    info->samples = samples;
    memcpy(info->buffer, pcm, info->samples*sizeof(mp3d_sample_t));
    /* save info */
    info->channels = frame_info.channels;
    info->hz       = frame_info.hz;
    info->layer    = frame_info.layer;
    size_t avg_bitrate_kbps = frame_info.bitrate_kbps;
    size_t frames = 1;
    /* decode rest frames */
    int frame_bytes;
    do
    {
        if ((allocated - info->samples*sizeof(mp3d_sample_t)) < MINIMP3_MAX_SAMPLES_PER_FRAME*sizeof(mp3d_sample_t))
        {
            allocated *= 2;
            info->buffer = (mp3d_sample_t*)realloc(info->buffer, allocated);
        }
        samples = mp3dec_decode_frame(dec, buf, (int)buf_size, info->buffer + info->samples, &frame_info);
        frame_bytes = frame_info.frame_bytes;
        buf      += frame_bytes;
        buf_size -= frame_bytes;
        if (samples)
        {
            if (info->hz != frame_info.hz || info->layer != frame_info.layer)
                break;
            if (info->channels && info->channels != frame_info.channels)
#ifdef MINIMP3_ALLOW_MONO_STEREO_TRANSITION
                info->channels = 0; /* mark file with mono-stereo transition */
#else
                break;
#endif
            info->samples += samples*frame_info.channels;
            avg_bitrate_kbps += frame_info.bitrate_kbps;
            frames++;
            if (progress_cb)
                progress_cb(user_data, orig_buf_size, orig_buf_size - buf_size, &frame_info);
        }
    } while (frame_bytes);
    /* reallocate to normal buffer size */
    if (allocated != info->samples*sizeof(mp3d_sample_t))
        info->buffer = (mp3d_sample_t*)realloc(info->buffer, info->samples*sizeof(mp3d_sample_t));
    info->avg_bitrate_kbps = (int)(avg_bitrate_kbps/frames);
}

void mp3dec_iterate_buf(const uint8_t *buf, size_t buf_size, MP3D_ITERATE_CB callback, void *user_data)
{
    if (!callback)
        return;
    mp3dec_frame_info_t frame_info;
    memset(&frame_info, 0, sizeof(frame_info));
    /* skip id3v2 */
    size_t id3v2size = mp3dec_skip_id3v2(buf, buf_size);
    if (id3v2size > buf_size)
        return;
    const uint8_t *orig_buf = buf;
    buf      += id3v2size;
    buf_size -= id3v2size;
    do
    {
        int free_format_bytes = 0, frame_size = 0;
        int i = mp3d_find_frame(buf, (int)buf_size, &free_format_bytes, &frame_size);
        buf      += i;
        buf_size -= i;
        if (i && !frame_size)
            continue;
        if (!frame_size)
            break;
        const uint8_t *hdr = buf;
        frame_info.channels = HDR_IS_MONO(hdr) ? 1 : 2;
        frame_info.hz = hdr_sample_rate_hz(hdr);
        frame_info.layer = 4 - HDR_GET_LAYER(hdr);
        frame_info.bitrate_kbps = hdr_bitrate_kbps(hdr);
        frame_info.frame_bytes = frame_size;

        if (callback(user_data, hdr, frame_size, hdr - orig_buf, &frame_info))
            break;
        buf      += frame_size;
        buf_size -= frame_size;
    } while (1);
}

int mp3dec_ex_open_buf(mp3dec_ex_t *dec, const uint8_t *buf, size_t buf_size, int seek_method)
{
    memset(dec, 0, sizeof(*dec));
    dec->file.buffer = buf;
    dec->file.size   = buf_size;
    dec->seek_method = seek_method;
    mp3dec_init(&dec->mp3d);
    return 0;
}

/*void mp3dec_ex_seek(mp3dec_ex_t *dec, size_t position)
{
}

int mp3dec_ex_read(mp3dec_ex_t *dec, int16_t *buf, int samples)
{
    return 0;
}*/

#ifndef MINIMP3_NO_STDIO

#if defined(__linux__) || defined(__FreeBSD__)
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#if !defined(MAP_POPULATE) && defined(__linux__)
#define MAP_POPULATE 0x08000
#elif !defined(MAP_POPULATE)
#define MAP_POPULATE 0
#endif

static void mp3dec_close_file(mp3dec_map_info_t *map_info)
{
    if (map_info->buffer && MAP_FAILED != map_info->buffer)
        munmap((void *)map_info->buffer, map_info->size);
    map_info->buffer = 0;
    map_info->size   = 0;
}

static int mp3dec_open_file(const char *file_name, mp3dec_map_info_t *map_info)
{
    int file;
    struct stat st;
    memset(map_info, 0, sizeof(*map_info));
retry_open:
    file = open(file_name, O_RDONLY);
    if (file < 0 && (errno == EAGAIN || errno == EINTR))
        goto retry_open;
    if (file < 0 || fstat(file, &st) < 0)
    {
        close(file);
        return -1;
    }

    map_info->size = st.st_size;
retry_mmap:
    map_info->buffer = (const uint8_t *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, file, 0);
    if (MAP_FAILED == map_info->buffer && (errno == EAGAIN || errno == EINTR))
        goto retry_mmap;
    close(file);
    if (MAP_FAILED == map_info->buffer)
        return -1;
    return 0;
}
#elif defined(_WIN32)
#include <windows.h>

static void mp3dec_close_file(mp3dec_map_info_t *map_info)
{
    if (map_info->buffer)
        UnmapViewOfFile(map_info->buffer);
    map_info->buffer = 0;
    map_info->size = 0;
}

static int mp3dec_open_file(const char *file_name, mp3dec_map_info_t *map_info)
{
    memset(map_info, 0, sizeof(*map_info));

    HANDLE mapping = NULL;
    HANDLE file = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (INVALID_HANDLE_VALUE == file)
        return -1;
    LARGE_INTEGER s;
    s.LowPart = GetFileSize(file, (DWORD*)&s.HighPart);
    if (s.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
        goto error;
    map_info->size = (size_t)s.QuadPart;

    mapping = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!mapping)
        goto error;
    map_info->buffer = (const uint8_t*) MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, (SIZE_T)s.QuadPart);
    CloseHandle(mapping);
    if (!map_info->buffer)
        goto error;

    CloseHandle(file);
    return 0;
error:
    mp3dec_close_file(map_info);
    CloseHandle(file);
    return -1;
}
#else
#include <stdio.h>

static void mp3dec_close_file(mp3dec_map_info_t *map_info)
{
    if (map_info->buffer)
        free((void *)map_info->buffer);
    map_info->buffer = 0;
    map_info->size = 0;
}

static int mp3dec_open_file(const char *file_name, mp3dec_map_info_t *map_info)
{
    memset(map_info, 0, sizeof(*map_info));
    FILE *file = fopen(file_name, "rb");
    if (!file)
        return -1;
    long size = -1;
    if (fseek(file, 0, SEEK_END))
        goto error;
    size = ftell(file);
    if (size < 0)
        goto error;
    map_info->size = (size_t)size;
    if (fseek(file, 0, SEEK_SET))
        goto error;
    map_info->buffer = (uint8_t *)malloc(map_info->size);
    if (!map_info->buffer)
        goto error;
    if (fread((void *)map_info->buffer, 1, map_info->size, file) != map_info->size)
        goto error;
    fclose(file);
    return 0;
error:
    mp3dec_close_file(map_info);
    fclose(file);
    return -1;
}
#endif

int mp3dec_load(mp3dec_t *dec, const char *file_name, mp3dec_file_info_t *info, MP3D_PROGRESS_CB progress_cb, void *user_data)
{
    int ret;
    mp3dec_map_info_t map_info;
    if ((ret = mp3dec_open_file(file_name, &map_info)))
        return ret;
    mp3dec_load_buf(dec, map_info.buffer, map_info.size, info, progress_cb, user_data);
    mp3dec_close_file(&map_info);
    return info->samples ? 0 : -1;
}

int mp3dec_iterate(const char *file_name, MP3D_ITERATE_CB callback, void *user_data)
{
    int ret;
    mp3dec_map_info_t map_info;
    if ((ret = mp3dec_open_file(file_name, &map_info)))
        return ret;
    mp3dec_iterate_buf(map_info.buffer, map_info.size, callback, user_data);
    mp3dec_close_file(&map_info);
    return 0;
}

void mp3dec_ex_close(mp3dec_ex_t *dec)
{
    if (dec->is_file)
        mp3dec_close_file(&dec->file);
    else
        free((void *)dec->file.buffer);
    memset(dec, 0, sizeof(*dec));
}

int mp3dec_ex_open(mp3dec_ex_t *dec, const char *file_name, int seek_method)
{
    int ret;
    memset(dec, 0, sizeof(*dec));
    if ((ret = mp3dec_open_file(file_name, &dec->file)))
        return ret;
    dec->seek_method = seek_method;
    dec->is_file = 1;
    mp3dec_init(&dec->mp3d);
    return 0;
}
#else
void mp3dec_ex_close(mp3dec_ex_t *dec)
{
    free((void*)dec->file.buffer);
    memset(dec, 0, sizeof(*dec));
}
#endif

#endif /*MINIMP3_IMPLEMENTATION*/
