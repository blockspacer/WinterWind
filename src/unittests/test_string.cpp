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

#include "test_string.h"

#include <core/utils/base64.h>
#include <core/utils/stringutils.h>
#include <core/utils/hmac.h>

namespace winterwind {
namespace unittests {

void Test_String::str_hex()
{
	std::string orig = "hello hex to convert.";
	std::string dest;
	str_to_hex(orig, dest);
	CPPUNIT_ASSERT(dest == "68656c6c6f2068657820746f20636f6e766572742e");
}

void Test_String::split_string()
{
	std::string orig = "hello, this is winterwind.";
	std::vector<std::string> res;
	str_split(orig, ' ', res);
	CPPUNIT_ASSERT(res.size() == 4 && res[2] == "is");
}

void Test_String::remove_substring()
{
	std::string orig = "The world is mine, the world is not yours";
	std::string to_alter = orig;
	str_remove_substr(to_alter, "world ");
	CPPUNIT_ASSERT(to_alter == "The is mine, the is not yours");
}

void Test_String::base64_encode_test()
{
	std::string src = "unittest_b64encode";
	std::string res = base64_encode((const unsigned char *) src.c_str(),
		static_cast<unsigned int>(src.size()));
	CPPUNIT_ASSERT(res.compare("dW5pdHRlc3RfYjY0ZW5jb2Rl") == 0);
}

void Test_String::base64_decode_test()
{
	std::string src = "dW5pdHRlc3RfYjY0ZGVjb2Rl";
	std::string res = base64_decode(src);
	CPPUNIT_ASSERT(res.compare("unittest_b64decode") == 0);
}

void Test_String::base64_encode_test2()
{
	std::string src = "unittest_b64encode";
	std::string res = base64_encode(src);
	CPPUNIT_ASSERT(res.compare("dW5pdHRlc3RfYjY0ZW5jb2Rl") == 0);
}

void Test_String::base64_urlencode_test()
{
	std::string src = "unittest_b64urlencode / me";
	std::string res = base64_urlencode((const unsigned char *) src.c_str(),
		static_cast<unsigned int>(src.size()));
	CPPUNIT_ASSERT(res.compare("dW5pdHRlc3RfYjY0dXJsZW5jb2RlIC8gbWU=") == 0);
}

void Test_String::base64_urldecode_test()
{
	std::string src = "dW5pdHRlc3RfYjY0dXJsZGVjb2RlIC8gbWU=";
	std::string res = base64_urldecode(src);

	static const std::string req_result = "unittest_b64urldecode / me";

	CPPUNIT_ASSERT_MESSAGE(res + " != " + req_result, res.compare(req_result) == 0);
}

void Test_String::base64_urlencode_test2()
{
	std::string src = "unittest_b64urlencode / me";
	std::string res = base64_urlencode(src);
	CPPUNIT_ASSERT(res.compare("dW5pdHRlc3RfYjY0dXJsZW5jb2RlIC8gbWU=") == 0);
}

void Test_String::hmac_md5_test()
{
	static const std::string expected_result = "P4Dt/+VJbIzei9pPGqWybQ==";
	std::string res = base64_encode(hmac_md5("unittest_key_5", "hashthatmd5"));
	std::string message = res + " != '" + expected_result + "'";
	CPPUNIT_ASSERT_MESSAGE(message, res == expected_result);
}

void Test_String::hmac_sha1_test()
{
	static const std::string expected_result = "rfsumIkQ/lUjuI68D1t0eJe/PgE=";
	std::string res = base64_encode(hmac_sha1("unittest_key", "hashthatthing"));
	std::string message = res + " != '" + expected_result + "'";
	CPPUNIT_ASSERT_MESSAGE(message, res == expected_result);
}

void Test_String::hmac_sha256_test()
{
	static const std::string expected_result = "gcYLiiayTR4BW+X4U4WOnEYPM54Jv7iUwssb0EPYwWU=";
	std::string res = base64_encode(hmac_sha256("unittest_key_256", "hashthat256thing"));
	std::string message = res + " != '" + expected_result + "'";
	CPPUNIT_ASSERT_MESSAGE(message, res == expected_result);
}

void Test_String::hmac_sha384_test()
{
	static const std::string expected_result = "Z1YKhbAW6ZLo5+wywZvFHR3SskgMApO5C8DKmA/htbBaahCfuIDhj3j6ANue9JST";
	std::string res = base64_encode(hmac_sha384("secret384", "hash384_message"));
	std::string message = res + " != '" + expected_result + "'";
	CPPUNIT_ASSERT_MESSAGE(message, res == expected_result);
}

void Test_String::hmac_sha512_test()
{
	static const std::string expected_result = "88YMIWdFi9oIlfwZp3uYzxXgxrcV2ToV29q0f5JC08Cq5tZ5FNSc3f+2iMQVIQpOYu8vtN0u5/kBgCXpmkLqBg==";
	std::string res = base64_encode(hmac_sha512("secret512", "hash512_message"));
	std::string message = res + " != '" + expected_result + "'";
	CPPUNIT_ASSERT_MESSAGE(message, res == expected_result);
}

void Test_String::count_words_test()
{
	static const std::string to_count = "Red is rose as purple is blue";
	static const uint16_t expected_count = 7;
	CPPUNIT_ASSERT(count_words(to_count) == expected_count);
}

void Test_String::replace_str()
{
	std::string orig = "Red is rose as purple is blue";
	static const std::string repl_expected = "Red is rose as purple is black";
	static const std::string repl = "blue";
	static const std::string repl_with = "black";

	CPPUNIT_ASSERT(replace(orig, repl, repl_with));
	CPPUNIT_ASSERT(orig == repl_expected);
}

void Test_String::trim_test()
{
	static const std::vector<std::string> trim_start = {
		"",
		"test_0",
		"test_1 \t\f \n  \t ",
		"  \n\f  test_2",
		"\ntest_3\t",
		" \n\f test\n_4\t  "
	};

	static const std::vector<std::string> trim_expected = {
		"",
		"test_0",
		"test_1",
		"test_2",
		"test_3",
		"test\n_4"
	};

	CPPUNIT_ASSERT_MESSAGE("trim_start size != trim_expected size",
		trim_start.size() == trim_expected.size());

	for (uint8_t i = 0; i < trim_start.size(); i++) {
		std::string to_trim = trim_start[i];
		trim(to_trim);
		CPPUNIT_ASSERT_MESSAGE("trim expected: '" + trim_expected[i]
			+ "' but found: '" + to_trim + "'", to_trim == trim_expected[i]);
	}
}
}
}
