/*
 *    decoder.cpp
 *
 *    Copyright 2013 Dominique Hunziker
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 *     \author/s: Dominique Hunziker
 */

#include "libav_image_transport/decoder.hpp"

#include <iostream>
#include <boost/make_shared.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

extern "C"
{
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include "libav_image_transport/config.hpp"
#include "libav_image_transport/pix_fmt.hpp"

#ifdef BACKPORT_LIBAV
#include "libav_image_transport/libav_backport.hpp"
#endif

namespace libav_image_transport
{

void Decoder::init_decoder(const int in_width, const int in_height,
		const int in_pix_fmt, const int codec_ID)
{
	/* Declarations */
	AVCodec *codec;

	/* Find the decoder */
	codec = find_decoder(codec_ID);
	if (!codec)
		throw std::runtime_error("Could not find codec.");

	/* Allocate codec context */
	codec_context_ = avcodec_alloc_context3(codec);
	if (!codec_context_)
		throw std::runtime_error("Could not allocate codec context.");

	/* Set codec configuration */
	codec_context_->pix_fmt = (enum AVPixelFormat) in_pix_fmt;
	codec_context_->width = in_width;
	codec_context_->height = in_height;

	/* Open the codec context */
	if (codec_open(codec_context_, codec, NULL) < 0)
		throw std::runtime_error("Could not open codec context.");

	/* Allocate Decoder AVFrame */
	frame_in_ = boost::make_shared<Frame>(in_width, in_height, in_pix_fmt);

	has_keyframe_ = false;
	previous_packet_ = 0;
}

void Decoder::decode(const Packet::ConstPtr &packet,
		sensor_msgs::ImagePtr& image, int &got_image)
{
	/* Declarations */
	int size;
	AVPacket pkt;
	AVFrame* frame_in;
	AVFrame* frame_out;

	/* Check if the codec context has to be reinitialized */
	if (!codec_context_ || packet->codec_ID != codec_context_->codec_id
			|| packet->compressed_pix_fmt != codec_context_->pix_fmt
			|| packet->compressed_width != codec_context_->width
			|| packet->compressed_height != codec_context_->height)
	{
		free_context();
		init_decoder(packet->compressed_width, packet->compressed_height,
				packet->compressed_pix_fmt, packet->codec_ID);
	}

	/* Get local references to the AVFrame structs */
	frame_in = frame_in_->get_frame();
	frame_out = frame_out_ ? frame_out_->get_frame() : NULL;

	/* Check if the output frame has to be reinitialized */
	if (!frame_out || frame_out->width != packet->width
			|| frame_out->height != packet->height
			|| frame_out->format != packet->pix_fmt)
	{
		frame_out_ = boost::make_shared<Frame>(packet->width, packet->height,
				packet->pix_fmt);
		frame_out = frame_out_->get_frame();
	}

	/* Check if the received packet is valid */
	if (previous_packet_ + 1 != packet->packet_number)
	{
		has_keyframe_ = false;
		previous_packet_ = 0;
	}

	previous_packet_ = packet->packet_number;

	/* Check if there is a valid keyframe stored */
	if (!has_keyframe_)
	{
		if (packet->keyframe)
			has_keyframe_ = true;
		else
		{
			got_image = 0;
			return;
		}
	}

	/* Fill the AVPacket */
	if (av_new_packet(&pkt, packet->data.size()))
		throw std::runtime_error("Could not allocate AV packet data.");

	memcpy(pkt.data, &packet->data[0], packet->data.size());

	/* Decode packet */
	if (avcodec_decode_video2(codec_context_, frame_in, &got_image, &pkt) < 0)
		std::cout << "[decode] Could no decode packet." << std::endl;

	/* Free the packet data */
	av_free_packet(&pkt);

	if (got_image)
	{
		/* Get SWS Context */
		sws_context_ = sws_getCachedContext(sws_context_, frame_in->width,
				frame_in->height, (enum AVPixelFormat) frame_in->format,
				frame_out->width, frame_out->height,
				(enum AVPixelFormat) frame_out->format, SWS_BICUBIC, NULL, NULL,
				NULL);
		if (!sws_context_)
			throw std::runtime_error("Could not initialize sws context.");

		/* Transform image */
		sws_scale(sws_context_, (const uint8_t* const *) frame_in->data,
				frame_in->linesize, 0, frame_in->height, frame_out->data,
				frame_out->linesize);

		/* Intermediate results used ot store image */
		size = frame_out->linesize[0] * frame_out->height;
		boost::posix_time::time_duration pts(0, 0, 0,
				frame_in->pkt_pts * (pts.ticks_per_second() / 1.e9));

		/* Store image */
		image->header.seq = packet->packet_number;
		image->header.stamp = ros::Time::fromBoost(
				packet->reference.toBoost() + pts);
		image->width = frame_out->width;
		image->height = frame_out->height;
		image->step = frame_out->linesize[0];

		if (!pix_fmt_libav2ros(frame_out->format, image->encoding,
				image->is_bigendian))
			throw std::runtime_error(
					"Can not handle requested output pixel format.");

		image->data.resize(size);
		image->data.assign(frame_out->data[0], frame_out->data[0] + size);
	}
}

} /* namespace libav_image_transport */
