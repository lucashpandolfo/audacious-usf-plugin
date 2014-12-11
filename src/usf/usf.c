
#include <stdint.h>
#include "usf.h"
#include "cpu.h"
#include "memory.h"
#include "audio.h"
#include "psftag.h"

#include <stdio.h>
#include <stdlib.h>

#include <audacious/plugin.h>

#include <glib.h>

#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <inttypes.h>

#include "types.h"

extern int SampleRate;

extern InputPlugin usf_ip;
InputPlayback *pcontext = 0;

void usf_mseek(InputPlayback * context, gint millisecond);
int8_t filename[512];
uint32_t cpu_running = 0, use_interpreter = 0, use_audiohle =
    0, is_paused = 0, cpu_stopped = 1, fake_seek_stopping = 0;
uint32_t is_fading = 0, fade_type = 1, fade_time = 5000, is_seeking =
    0, seek_backwards = 0, track_time = 180000;
double seek_time = 0.0, play_time = 0.0, rel_volume = 1.0;

uint32_t enablecompare = 0, enableFIFOfull = 0;

uint32_t usf_length = 0, usf_fade_length = 0;

int8_t title[100];
uint8_t title_format[] = "%game% - %title%";

extern int32_t RSP_Cpu;

uint32_t get_length_from_string(uint8_t * str_length)
{
    uint32_t ttime = 0, temp = 0, mult = 1, level = 1;
    char Source[1024];
    uint8_t *src = (uint8_t *) (Source + strlen((char *) str_length));
    strcpy(&Source[1], (char *) str_length);
    Source[0] = 0;

    while (*src) {
	if ((*src >= '0') && (*src <= '9')) {
	    temp += ((*src - '0') * mult);
	    mult *= 10;
	} else {
	    mult = 1;
	    if (*src == '.') {
		ttime = temp;
		temp = 0;
	    } else if (*src == ':') {
		ttime += (temp * (1000 * level));
		temp = 0;
		level *= 60;
	    }
	}
	src--;
    }

    ttime += (temp * (1000 * level));
    return ttime;
}

void usf_fread(void *ptr, int64_t size, int64_t nmemb, VFSFile * file)
{
    int64_t ret;
    ret = vfs_fread(ptr, size, nmemb, file);
    if (ret != nmemb)
	printf
	    ("USF: Warning, tried reading %ld element(s) but vfs_read returned %ld\n",
	     nmemb, ret);
}

void usf_fseek(VFSFile * file, int64_t offset, int whence)
{
    int ret;
    ret = vfs_fseek(file, offset, whence);
    if (ret != 0)
	printf("USF: Warning, vfs_seek returned %d instead of zero\n", ret);
}




int LoadUSF(const gchar * fn, VFSFile * fil)
{
    uint32_t reservedsize = 0, codesize = 0, crc = 0, tagstart =
	0, reservestart = 0;
    uint32_t filesize = 0, tagsize = 0, temp = 0;
    uint8_t buffer[16], *buffer2 = NULL, *tagbuffer = NULL;

    is_fading = 0;
    fade_type = 1;
    fade_time = 5000;
    track_time = 180000;
    play_time = 0;
    is_seeking = 0;
    seek_backwards = 0;
    seek_time = 0;

    if (!fil) {
	printf("USF: Could not open USF!\n");
	return 0;
    }

    usf_fread(buffer, 4, 1, fil);
    if (buffer[0] != 'P' && buffer[1] != 'S' && buffer[2] != 'F'
	&& buffer[3] != 0x21) {
	printf("USF: Invalid header in file!\n");
	return 0;
    }

    usf_fread(&reservedsize, 4, 1, fil);
    usf_fread(&codesize, 4, 1, fil);
    usf_fread(&crc, 4, 1, fil);

    usf_fseek(fil, 0, SEEK_END);
    filesize = vfs_ftell(fil);

    reservestart = 0x10;
    tagstart = reservestart + reservedsize;
    tagsize = filesize - tagstart;

    if (tagsize) {
	usf_fseek(fil, tagstart, SEEK_SET);
	usf_fread(buffer, 5, 1, fil);

	if (buffer[0] != '[' && buffer[1] != 'T' && buffer[2] != 'A'
	    && buffer[3] != 'G' && buffer[4] != ']') {
	    printf("USF: Erroneous data in tag area! %" PRIu32 "\n", tagsize);
	    return 0;
	}

	buffer2 = malloc(50001);
	tagbuffer = malloc(tagsize);

	usf_fread(tagbuffer, tagsize, 1, fil);

	psftag_raw_getvar((char *) tagbuffer, "_lib", (char *) buffer2,
			  50000);

	if (strlen((char *) buffer2)) {
	    char path[512];
	    int pathlength = 0;

	    if (strrchr(fn, '/'))	//linux
		pathlength = strrchr(fn, '/') - fn + 1;
	    else if (strrchr(fn, '\\'))	//windows
		pathlength = strrchr(fn, '\\') - fn + 1;
	    else		//no path
		pathlength = strlen(fn);

	    strncpy(path, fn, pathlength);
	    path[pathlength] = 0;
	    strcat(path, (char *) buffer2);

	    VFSFile *file2 = vfs_fopen(path, "rb");
	    LoadUSF(path, file2);
	    vfs_fclose(file2);
	}

	psftag_raw_getvar((char *) tagbuffer, "_enablecompare",
			  (char *) buffer2, 50000);
	if (strlen((char *) buffer2))
	    enablecompare = 1;
	else
	    enablecompare = 0;

	psftag_raw_getvar((char *) tagbuffer, "_enableFIFOfull",
			  (char *) buffer2, 50000);
	if (strlen((char *) buffer2))
	    enableFIFOfull = 1;
	else
	    enableFIFOfull = 0;

	psftag_raw_getvar((char *) tagbuffer, "length", (char *) buffer2,
			  50000);
	if (strlen((char *) buffer2)) {
	    track_time = get_length_from_string(buffer2);
	}

	psftag_raw_getvar((char *) tagbuffer, "fade", (char *) buffer2,
			  50000);
	if (strlen((char *) buffer2)) {
	    fade_time = get_length_from_string(buffer2);
	}

	psftag_raw_getvar((char *) tagbuffer, "title", (char *) buffer2,
			  50000);
	if (strlen((char *) buffer2))
	    strcpy((char *) title, (char *) buffer2);
	else {
	    int pathlength = 0;

	    if (strrchr(fn, '/'))	//linux
		pathlength = strrchr(fn, '/') - fn + 1;
	    else if (strrchr(fn, '\\'))	//windows
		pathlength = strrchr(fn, '\\') - fn + 1;
	    else		//no path
		pathlength = 7;

	    strcpy((char *) title, &fn[pathlength]);

	}

	free(buffer2);
	buffer2 = NULL;

	free(tagbuffer);
	tagbuffer = NULL;

    }

    usf_fseek(fil, reservestart, SEEK_SET);
    usf_fread(&temp, 4, 1, fil);

    if (temp == 0x34365253) {	//there is a rom section
	int len = 0, start = 0;
	usf_fread(&len, 4, 1, fil);

	while (len) {
	    usf_fread(&start, 4, 1, fil);

	    while (len) {
		int page = start >> 16;
		int readLen =
		    (((start + len) >> 16) >
		     page) ? (((page + 1) << 16) - start) : len;

		if (ROMPages[page] == 0) {
		    ROMPages[page] = malloc(0x10000);
		    memset(ROMPages[page], 0, 0x10000);
		}

		usf_fread(ROMPages[page] + (start & 0xffff), readLen, 1,
			  fil);

		start += readLen;
		len -= readLen;
	    }

	    usf_fread(&len, 4, 1, fil);
	}

    }



    usf_fread(&temp, 4, 1, fil);
    if (temp == 0x34365253) {
	int len = 0, start = 0;
	usf_fread(&len, 4, 1, fil);

	while (len) {
	    usf_fread(&start, 4, 1, fil);

	    usf_fread(savestatespace + start, len, 1, fil);

	    usf_fread(&len, 4, 1, fil);
	}
    }
    // Detect the Ramsize before the memory allocation

    if (*(uint32_t *) (savestatespace + 4) == 0x400000) {
	RdramSize = 0x400000;
	savestatespace = realloc(savestatespace, 0x40275c);
    } else if (*(uint32_t *) (savestatespace + 4) == 0x800000)
	RdramSize = 0x800000;

    return 1;
}


gboolean usf_init()
{
    use_audiohle = 0;
    use_interpreter = 0;
    RSP_Cpu = 0;		// 0 is recompiler, 1 is interpreter

    return TRUE;
}

void usf_destroy()
{

}

void usf_seek(InputPlayback * context, gint time)
{
    usf_mseek(context, time * 1000);
}


void usf_mseek(InputPlayback * context, gint millisecond)
{
    if (millisecond < play_time) {
	is_paused = 0;

	fake_seek_stopping = 1;
	CloseCpu();

	while (!cpu_stopped)
	    usleep(1);

	is_seeking = 1;
	seek_time = (double) millisecond;

	fake_seek_stopping = 2;
    } else {
	is_seeking = 1;
	seek_time = (double) millisecond;
    }

    context->output->flush(millisecond / 1000);
}

gboolean usf_playing;

gboolean usf_play(InputPlayback * context, const gchar * filename,
		  VFSFile * file, gint start_time, gint stop_time,
		  gboolean pause)
{
    // Defaults (which would be overriden by Tags / playing
    savestatespace = NULL;
    cpu_running = is_paused = fake_seek_stopping = 0;
    cpu_stopped = 1;
    is_fading = 0;
    fade_type = 1;
    fade_time = 5000;
    is_seeking = 0;
    seek_backwards = 0;
    track_time = 180000;
    seek_time = 0.0;
    play_time = 0.0;
    rel_volume = 1.0;


    pcontext = context;

    // Allocate main memory after usf loads  (to determine ram size)

    PreAllocate_Memory();

    if (!LoadUSF(filename, file)) {
	Release_Memory();
	vfs_fclose(file);
	return FALSE;
    }

    context->set_pb_ready(context);

    Allocate_Memory();

    usf_playing = TRUE;
    while (usf_playing) {
	is_fading = 0;
	play_time = 0;

	StartEmulationFromSave(savestatespace);
	if (!fake_seek_stopping)
	    break;
	while (fake_seek_stopping != 2)
	    usleep(1);
	fake_seek_stopping = 4;
    }

    Release_Memory();



    return TRUE;
}

void usf_stop(InputPlayback * context)
{
    while (cpu_running == 1) {
	cpu_running = 0;
	CloseCpu();
    }

    usf_playing = FALSE;
    is_paused = 0;
}

void usf_pause(InputPlayback * context, gboolean paused)
{
    is_paused = paused;
}

static const gchar *usf_exts[] = {
    "usf",
    "miniusf",
    NULL
};


Tuple *usf_get_song_tuple(const gchar * fn, VFSFile * fil)
{
    Tuple *tuple = NULL;
    uint32_t reservedsize = 0, codesize = 0, crc = 0, tagstart =
	0, reservestart = 0, filesize = 0, tagsize = 0;
    uint8_t buffer[16], *buffer2 = NULL, *tagbuffer = NULL;

    if (!fil) {
	printf("USF: Could not open USF!\n");
	return NULL;
    }

    usf_fread(buffer, 4, 1, fil);

    if (buffer[0] != 'P' && buffer[1] != 'S' && buffer[2] != 'F'
	&& buffer[3] != 0x21) {
	printf("USF: Invalid header in file!\n");
	return NULL;
    }

    usf_fread(&reservedsize, 4, 1, fil);
    usf_fread(&codesize, 4, 1, fil);
    usf_fread(&crc, 4, 1, fil);

    usf_fseek(fil, 0, SEEK_END);
    filesize = vfs_ftell(fil);

    reservestart = 0x10;
    tagstart = reservestart + reservedsize;
    tagsize = filesize - tagstart;

    tuple = tuple_new_from_filename(fn);

    if (tagsize) {
	int temp_fade = 0;
	usf_fseek(fil, tagstart, SEEK_SET);
	usf_fread(buffer, 5, 1, fil);

	if (buffer[0] != '[' && buffer[1] != 'T' && buffer[2] != 'A'
	    && buffer[3] != 'G' && buffer[4] != ']') {
	    printf("USF: Erroneous data in tag area! %" PRIu32 "\n", tagsize);
	    return NULL;
	}

	buffer2 = malloc(50001);
	tagbuffer = malloc(tagsize);

	usf_fread(tagbuffer, tagsize, 1, fil);

	psftag_raw_getvar((char *) tagbuffer, "fade", (char *) buffer2,
			  50000);
	if (strlen((char *) buffer2))
	    temp_fade = get_length_from_string(buffer2);

	psftag_raw_getvar((char *) tagbuffer, "length", (char *) buffer2,
			  50000);
	if (strlen((char *) buffer2))
	    tuple_set_int(tuple, FIELD_LENGTH, NULL,
			  get_length_from_string(buffer2) + temp_fade);
	else
	    tuple_set_int(tuple, FIELD_LENGTH, NULL, (180 * 1000));

	psftag_raw_getvar((char *) tagbuffer, "title", (char *) buffer2,
			  50000);
	if (strlen((char *) buffer2))
	    tuple_set_str(tuple, FIELD_TITLE, NULL, (char *) buffer2);
	else {
	    char title[512];
	    int pathlength = 0;

	    if (strrchr(fn, '/'))	//linux
		pathlength = strrchr(fn, '/') - fn + 1;
	    else if (strrchr(fn, '\\'))	//windows
		pathlength = strrchr(fn, '\\') - fn + 1;
	    else		//no path
		pathlength = 7;

	    strcpy(title, &fn[pathlength]);

	    tuple_set_str(tuple, FIELD_TITLE, NULL, title);

	}

	psftag_raw_getvar((char *) tagbuffer, "artist", (char *) buffer2,
			  50000);
	if (strlen((char *) buffer2))
	    tuple_set_str(tuple, FIELD_ARTIST, NULL, (char *) buffer2);

	psftag_raw_getvar((char *) tagbuffer, "game", (char *) buffer2,
			  50000);
	if (strlen((char *) buffer2)) {
	    tuple_set_str(tuple, FIELD_ALBUM, NULL, (char *) buffer2);
	    tuple_set_str(tuple, -1, "game", (char *) buffer2);
	}

	psftag_raw_getvar((char *) tagbuffer, "copyright",
			  (char *) buffer2, 50000);
	if (strlen((char *) buffer2))
	    tuple_set_str(tuple, FIELD_COPYRIGHT, NULL, (char *) buffer2);

	// This for unknown reasons turns the "Kbps" in the UI to "channels"
	//tuple_set_str(tuple, FIELD_QUALITY, NULL, "sequenced");

	tuple_set_str(tuple, FIELD_CODEC, NULL, "Nintendo 64 Audio");
	tuple_set_str(tuple, -1, "console", "Nintendo 64");

	free(tagbuffer);
	free(buffer2);
    } else {
	char title[512];
	int pathlength = 0;

	if (strrchr(fn, '/'))	//linux
	    pathlength = strrchr(fn, '/') - fn + 1;
	else if (strrchr(fn, '\\'))	//windows
	    pathlength = strrchr(fn, '\\') - fn + 1;
	else			//no path
	    pathlength = 7;

	strcpy(title, &fn[pathlength]);


	tuple_set_int(tuple, FIELD_LENGTH, NULL, (180 * 1000));
	tuple_set_str(tuple, FIELD_TITLE, NULL, title);
    }

    return tuple;
}

AUD_INPUT_PLUGIN
    (.name = "USF Audio",.init = usf_init,.play = usf_play,.stop =
     usf_stop,.pause = usf_pause,.mseek = usf_mseek,.extensions =
     usf_exts,.probe_for_tuple = usf_get_song_tuple,)
