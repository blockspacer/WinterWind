/**
 * Copyright (c) 2016, Loic Blot <loic.blot@unix-experience.fr>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 * and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <chrono>
#include <cstring>
#include <sstream>
#include <string>
#include <algorithm>

#define NOW std::chrono::system_clock::now()
#define START_CHRONO const std::chrono::time_point<std::chrono::system_clock> start_time = NOW;
#define END_CHRONO const std::chrono::time_point<std::chrono::system_clock> end_time = NOW;

inline void chrono_duration(const std::chrono::time_point<std::chrono::system_clock> &start_time,
			    const std::chrono::time_point<std::chrono::system_clock> &end_time, double &result) {
	const std::chrono::duration<double> elapsed_seconds = end_time - start_time;
	result = elapsed_seconds.count();
}

#define PERIODIC_FUNCTION(F, TV, T, I)                                                                                 \
	T -= I;                                                                                                        \
	if (T <= I) {                                                                                                  \
		F;                                                                                                     \
		T = TV;                                                                                                \
	}

inline std::string readable_chrono_duration(std::chrono::time_point<std::chrono::system_clock> start_time,
					    std::chrono::time_point<std::chrono::system_clock> end_time) {
	double elapsed_seconds;
	chrono_duration(start_time, end_time, elapsed_seconds);
	std::stringstream ss;
	if (elapsed_seconds * 1000.f < 1.0f) {
		ss << elapsed_seconds * 1000.f * 1000.0f << "µs";
	} else if (elapsed_seconds < 1.0f) {
		ss << elapsed_seconds * 1000.f << "ms";
	} else {
		ss << elapsed_seconds << "s";
	}

	return ss.str();
}

static void timestamp_str_hour(const time_t t, std::string &res) {
	std::tm *ptm = std::localtime(&t);
	char buffer[10] = {0};
	std::strftime(buffer, sizeof(buffer), "%H:%M:%S", ptm);
	res = std::string(buffer, strlen(buffer));
}

static void time_to_string(const time_t &t, std::string &res, const bool gmt = false) {
	std::tm *ptm = (gmt ? std::gmtime(&t) : std::localtime(&t));
	char buffer[24] = {0};
	std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", ptm);
	res = std::string(buffer, strlen(buffer));
}

static void time_now_string(std::string &res) { time_to_string(time(NULL), res); }

static bool str_to_time(std::string str, std::tm &t)
{
	std::replace(str.begin() + 9, str.end() - 8, 'T', ' ');
	std::replace(str.end() - 2, str.end(), 'Z', '\0');
	std::istringstream ss(str);
	ss >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");
	return !ss.fail();
}

static bool str_to_timestamp(const std::string &str, std::time_t &t)
{
	std::tm _tm = {};
	bool r = str_to_time(str, _tm);
	t = std::move(std::mktime(&_tm));
	return r;
}

static int get_hour_now()
{
	time_t now = time(NULL);
	std::tm *ptm = std::localtime(&now);
	return ptm->tm_hour;
}

#define CHRONO_DURATION_STR readable_chrono_duration(start_time, end_time)
