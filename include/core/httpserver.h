/*
 * Copyright (c) 2016-2017, Loic Blot <loic.blot@unix-experience.fr>
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

#include "httpcommon.h"
#include "httpresponse.h"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <json/json.h>
#include <memory>
#include <microhttpd.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace winterwind {

enum HTTPQueryType
{
	HTTPQUERY_TYPE_NONE,
	HTTPQUERY_TYPE_FORM,
	HTTPQUERY_TYPE_JSON,
};

struct HTTPQuery
{
	std::string url = "";
	std::unordered_map<std::string, std::string> headers;
	std::unordered_map<std::string, std::string> get_params;

	virtual HTTPQueryType get_type() const { return HTTPQUERY_TYPE_NONE; }
};

struct HTTPFormQuery : public HTTPQuery
{
	std::unordered_map<std::string, std::string> post_data;

	virtual HTTPQueryType get_type() const { return HTTPQUERY_TYPE_FORM; }
};

struct HTTPJsonQuery : public HTTPQuery
{
	Json::Value json_query;

	virtual HTTPQueryType get_type() const { return HTTPQUERY_TYPE_JSON; }
};

struct HTTPRequestSession
{
	std::string result = "";
	bool data_handled = false;
	uint32_t http_code = MHD_HTTP_OK;
};

typedef std::shared_ptr<HTTPQuery> HTTPQueryPtr;
typedef std::shared_ptr<HTTPResponse> HTTPResponsePtr;

typedef std::function<HTTPResponsePtr(const HTTPQueryPtr)> HTTPServerRequestHandler;

#define BIND_HTTPSERVER_HANDLER(s, m, u, hdl, obj)                                                                     \
    s->register_handler(HTTP_METHOD_##m, u, std::bind(hdl, obj, std::placeholders::_1));

typedef std::unordered_map<std::string, HTTPServerRequestHandler> HTTPServerReqHandlerMap;

class HTTPServer
{
public:
	HTTPServer(const uint16_t http_port);

	virtual ~HTTPServer();

	/**
	 * Register handler hdl for method & url
	 * This will permit to call it back when a request mathod method & url will be found.
	 *
	 * @param method HTTP method to match
	 * @param url URL to match
	 * @param hdl function pointer to handling
	 */
	void register_handler(HTTPMethod method, const std::string &url,
		const HTTPServerRequestHandler &hdl)
	{
		m_handlers[method][url] = hdl;
	}

	uint16_t get_port() const
	{ return m_http_port; }

private:
	static int mhd_iter_headers(void *cls, MHD_ValueKind kind, const char *key,
		const char *value);

	static int mhd_iter_getargs(void *cls, MHD_ValueKind kind, const char *key,
		const char *value);

	static int request_handler(void *http_server, MHD_Connection *connection,
		const char *url, const char *method, const char *version, const char *upload_data,
		size_t *upload_data_size, void **ptr);

	static void request_completed(void *cls, struct MHD_Connection *connection,
		void **con_cls, MHD_RequestTerminationCode toe);

	bool parse_post_data(const std::string &data, HTTPFormQuery *qf);

	bool handle_query(HTTPMethod m, MHD_Connection *conn, const std::string &url,
		const std::string &upload_data,
		HTTPRequestSession *session);

	/**
	 * MicroHTTPd service pointer
	 */
	MHD_Daemon *m_mhd_daemon = nullptr;

	/**
	 * Store handlers for each method & URL
	 */
	HTTPServerReqHandlerMap m_handlers[HTTP_METHOD_MAX];

	/**
	 * Listening port
	 */
	uint16_t m_http_port;
};

}
