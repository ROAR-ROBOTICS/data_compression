/*
 *    libav_image_subscriber.hpp
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

#ifndef LIBAV_IMAGE_TRANSPORT__LIBAV_IMAGE_SUBSCRIBER_HPP_
#define LIBAV_IMAGE_TRANSPORT__LIBAV_IMAGE_SUBSCRIBER_HPP_

#include "image_transport/simple_subscriber_plugin.h"

#include "libav_image_transport/Packet.h"

#include "libav_image_transport/worker.hpp"
#include "libav_image_transport/decoder.hpp"

namespace libav_image_transport
{

class LibAVImageSubscriber: public image_transport::SimpleSubscriberPlugin<
		libav_image_transport::Packet>
{
public:
	LibAVImageSubscriber(void)
	{
		worker_ = boost::make_shared<Worker>();
		worker_->resize(30);
		worker_->start();
		decoder_ = boost::make_shared<Decoder>();
	}

	virtual ~LibAVImageSubscriber(void)
	{
		worker_->stop();
	}

	virtual std::string getTransportName() const
	{
		return "libav_packet";
	}

protected:
	virtual void internalCallback(const Packet::ConstPtr &message,
			const Callback& user_cb);
	virtual void decode(const Packet::ConstPtr &packet,
			const Callback& user_cb);

	boost::shared_ptr<Worker> worker_;
	boost::shared_ptr<Decoder> decoder_;
};

} /* namespace libav_image_transport */

#endif /* LIBAV_IMAGE_TRANSPORT__LIBAV_IMAGE_SUBSCRIBER_HPP_ */
