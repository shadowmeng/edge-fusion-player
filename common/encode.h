/*
 * Muxing/encoding API; ffmpeg specific implementation is in encode_lavc.*.
 *
 * Copyright (C) 2011-2012 Rudolf Polzer <divVerent@xonotic.org>
 *
 * This file is part of mpv.
 *
 * mpv is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * mpv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with mpv.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MPLAYER_ENCODE_H
#define MPLAYER_ENCODE_H

#include <stdbool.h>

#include "demux/demux.h"

struct mpv_global;
struct mp_log;
struct encode_lavc_context;

struct encode_opts {
    char *file;
    char *format;
    char **fopts;
    float fps;
    float maxfps;
    char *vcodec;
    char **vopts;
    char *acodec;
    char **aopts;
    int harddup;
    float voffset;
    float aoffset;
    int copyts;
    int rawts;
    int autofps;
    int neverdrop;
    int video_first;
    int audio_first;
    int metadata;
};

// interface for mplayer.c
struct encode_lavc_context *encode_lavc_init(struct encode_opts *options,
                                             struct mpv_global *global);
void encode_lavc_finish(struct encode_lavc_context *ctx);
void encode_lavc_free(struct encode_lavc_context *ctx);
void encode_lavc_discontinuity(struct encode_lavc_context *ctx);
bool encode_lavc_showhelp(struct mp_log *log, struct encode_opts *options);
int encode_lavc_getstatus(struct encode_lavc_context *ctx, char *buf, int bufsize, float relative_position);
void encode_lavc_expect_stream(struct encode_lavc_context *ctx, int mt);
void encode_lavc_set_metadata(struct encode_lavc_context *ctx,
                              struct mp_tags *metadata);
void encode_lavc_set_video_fps(struct encode_lavc_context *ctx, float fps);
void encode_lavc_set_audio_pts(struct encode_lavc_context *ctx, double pts);
bool encode_lavc_didfail(struct encode_lavc_context *ctx); // check if encoding failed

#endif
