/*
 * MLP parser prototypes
 * Copyright (c) 2007 Ian Caulfield
 *
 * This file is part of Libav.
 *
 * Libav is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Libav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libav; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * MLP parser prototypes
 */

#ifndef AVCODEC_MLP_PARSER_H
#define AVCODEC_MLP_PARSER_H

#include "get_bits.h"

typedef struct MLPHeaderInfo
{
    int stream_type;            ///< 0xBB for MLP, 0xBA for TrueHD

    int group1_bits;            ///< The bit depth of the first substream
    int group2_bits;            ///< Bit depth of the second substream (MLP only)

    int group1_samplerate;      ///< Sample rate of first substream
    int group2_samplerate;      ///< Sample rate of second substream (MLP only)

    int channels_mlp;                       ///< Channel count for MLP streams
    int channels_thd_stream1;               ///< Channel count for substream 1 of TrueHD streams ("6-channel presentation")
    int channels_thd_stream2;               ///< Channel count for substream 2 of TrueHD streams ("8-channel presentation")
    uint64_t channel_layout_mlp;            ///< Channel layout for MLP streams
    uint64_t channel_layout_thd_stream1;    ///< Channel layout for substream 1 of TrueHD streams ("6-channel presentation")
    uint64_t channel_layout_thd_stream2;    ///< Channel layout for substream 2 of TrueHD streams ("8-channel presentation")

    int access_unit_size;       ///< Number of samples per coded frame
    int access_unit_size_pow2;  ///< Next power of two above number of samples per frame

    int is_vbr;                 ///< Stream is VBR instead of CBR
    int peak_bitrate;           ///< Peak bitrate for VBR, actual bitrate (==peak) for CBR

    int num_substreams;         ///< Number of substreams within stream
} MLPHeaderInfo;


int ff_mlp_read_major_sync(void *log, MLPHeaderInfo *mh, GetBitContext *gb);

#endif /* AVCODEC_MLP_PARSER_H */
