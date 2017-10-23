/*
* Copyright (c) 2017, Loic Blot <loic.blot@unix-experience.fr>
* All rights reserved.
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*   this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*   this list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <amqp.h>
#include <memory>

namespace winterwind
{
namespace amqp
{

class Message;

class Envelope
{
public:
	Envelope() = delete;
	explicit Envelope(amqp_envelope_t *envelope);
	~Envelope() = default;

	const std::string &get_consumer_tag() const
	{
		return m_consumer_tag;
	}

	uint64_t get_delivery_tag() const
	{
		return m_delivery_tag;
	}

	const std::string &get_exchange() const
	{
		return m_exchange;
	}

	bool is_redelivered() const
	{
		return m_redelivered;
	}

	const std::string &get_routing_key() const
	{
		return m_routing_key;
	}
private:
	std::shared_ptr<Message> m_message;
	std::string m_consumer_tag;
	uint64_t m_delivery_tag;
	std::string m_exchange;
	bool m_redelivered;
	std::string m_routing_key;
};
}
}