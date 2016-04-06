#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common/zarray.h"

#include "../imagesource/image_source.h"
#include <lcm/lcm.h>
#include "../lcmtypes/maebot_camera_data_t.h"

#define BUFSIZE (1024*1024)

int
main (int argc, char *argv[])
{
    char *url = NULL;

    if (argc > 1) {
        // if URL is provided on command line, use that.
        url = argv[1];
    }
    else {
        // otherwise, show all cameras and use the first one.

        zarray_t *urls = image_source_enumerate ();

        printf ("Cameras:\n");
        for (int i = 0; zarray_size (urls); i++) {
            char *url;
            zarray_get (urls, i, &url);
            printf ("  %3d: %s\n", i, url);
        }

        if (0==zarray_size (urls)) {
            printf ("Found no cameras.\n");
            return -1;
        }
        zarray_get (urls, 0, &url);
    }

    image_source_t *isrc = image_source_open (url);

    if (isrc == NULL) {
        printf ("Error opening device.\n");
        return -1;
    }

    for (int i = 0; i < isrc->num_formats (isrc); i++) {
        image_source_format_t ifmt;
        isrc->get_format (isrc, i, &ifmt);
        printf ("%3d: %4d x %4d (%s)\n",
                i, ifmt.width, ifmt.height, ifmt.format);
    }

    if (1) {
        int res = isrc->start (isrc);
        printf ("start: res = %d\n", res);
    }

    int nframes = 0;

//    setlinebuf(stdout);
	lcm_t * lcm = lcm_create(NULL);
	if(!lcm) {
		printf("lcm error\n");
		exit(1);
	}

    while(1) {
        image_source_data_t *frmd = calloc (1, sizeof(*frmd));
        int res = isrc->get_frame (isrc, frmd);
        if (res < 0) {
            printf ("get_frame fail: %d\n", res);
            continue;
        }
        else
            nframes++;
        maebot_camera_data_t cam_data;
        cam_data.utime = frmd->utime;
        cam_data.width = frmd->ifmt.width;
        cam_data.height = frmd->ifmt.height;
        cam_data.datalen = frmd->datalen;
        cam_data.data = calloc(frmd->datalen, sizeof(uint32_t));
        memcpy(cam_data.data, frmd->data, frmd->datalen);
        maebot_camera_data_t_publish(lcm, "CAMERA_DATA", &cam_data);

        printf ("get_frame: res = %d count = %10d (%10d bytes)\r",
                res, nframes, frmd->datalen);
        fflush (stdout);
        isrc->release_frame (isrc, frmd);
        free(cam_data.data);
    }

    return 0;
}

