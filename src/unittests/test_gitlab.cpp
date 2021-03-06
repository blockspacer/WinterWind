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

#include "test_gitlab.h"

#include <iomanip>
#include <regex>
#include <core/utils/time.h>

namespace winterwind {
namespace unittests {

std::string Test_Gitlab::RUN_TIMESTAMP = std::to_string(time(nullptr));

#define PRECHECK_GITLAB_CLIENT \
	CPPUNIT_ASSERT_MESSAGE("Gitlab client not inited due to configuration error", \
	m_gitlab_client.get() != nullptr);

#define ONLY_IF_GITLAB_INIT_SUCCEED \
if (gitlab_has_failed_in_init) { \
    return; \
}

#define MARK_GITLAB_FAILURE_ON_INIT_IF \
    if (m_gitlab_client->get_http_code() == 502) { \
    gitlab_has_failed_in_init = true; \
}

void Test_Gitlab::setUp()
{
	if (!getenv("GITLAB_API_KEY")) {
		std::cerr << __PRETTY_FUNCTION__ << ": Missing gitlab token" << std::endl;
		return;
	}

	m_gitlab_client = std::make_unique<GitlabAPIClient>("https://gitlab.com",
		std::string(getenv("GITLAB_API_KEY")));
}

void Test_Gitlab::tearDown()
{
	if (m_gitlab_client.get() == nullptr) {
		return;
	}

	// Cleanup very old projects
	Json::Value projects;
	if (m_gitlab_client->get_projects("", projects, GITLAB_PROJECT_SS_OWNED) ==
		GITLAB_RC_OK) {
		for (const auto &p: projects) {
			// Creation date is mandatory
			if (!p.isMember("created_at") || !p["created_at"].isString()) {
				continue;
			}

			std::time_t t;
			if (!str_to_timestamp(p["created_at"].asString(), t)) {
				std::cerr << "Failed to parse date: '" << p["create_at"].asString()
						  << "'" << std::endl;
				continue;
			}

			// Remove projects older than 24 hours
			if (t < std::time(0) - 86400) {
				std::cout << "Project " << p["name"].asString() << " will be deleted"
						  << std::endl;
				m_gitlab_client->delete_project(p["name"].asString());
			}

		}
	}

	Json::Value groups;
	if (m_gitlab_client->get_groups("ww_testgroup_", groups) == GITLAB_RC_OK) {
		std::regex re("ww_testgroup_([0-9]+)");
		for (const auto &g : groups) {
			std::smatch sm;
			const std::string gname = g["name"].asString();
			if (std::regex_match(gname, sm, re) && sm.size() > 0) {
				const std::string timestamp_str = sm.str(1);
				std::time_t t = std::atoi(timestamp_str.c_str());
				if (t < std::time(0) - 86400) {
					m_gitlab_client->delete_group(gname);
				}
			}
		}
	}
}

void Test_Gitlab::create_default_groups()
{
	PRECHECK_GITLAB_CLIENT;

	CPPUNIT_ASSERT_MESSAGE("Gitlab client not inited due to configuration error",
		m_gitlab_client.get() != nullptr);
	Json::Value res;
	GitlabGroup g("ww_testgroup_default_" + RUN_TIMESTAMP,
			"ww_testgroup_default_" + RUN_TIMESTAMP);

	bool rc = m_gitlab_client->create_group(g, res);
	std::string error_msg = std::string("Unable to create 1st default group (rc: ");
	error_msg += std::to_string(m_gitlab_client->get_http_code()) + ")";

	MARK_GITLAB_FAILURE_ON_INIT_IF

	CPPUNIT_ASSERT_MESSAGE(error_msg, rc || m_gitlab_client->get_http_code() == 502);

	ONLY_IF_GITLAB_INIT_SUCCEED

	GitlabGroup g2("ww_testgroup2_default_" + RUN_TIMESTAMP,
			"ww_testgroup2_default_" + RUN_TIMESTAMP);

	rc = m_gitlab_client->create_group(g2, res);

	MARK_GITLAB_FAILURE_ON_INIT_IF

	ONLY_IF_GITLAB_INIT_SUCCEED

	error_msg = std::string("Unable to create 2nd default group (rc: ");
	error_msg += std::to_string(m_gitlab_client->get_http_code()) + ")";
	CPPUNIT_ASSERT_MESSAGE(error_msg, rc);
}

void Test_Gitlab::create_group()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED

	Json::Value res;
	GitlabGroup g(TEST_GROUP, TEST_GROUP);
	g.description = "test";
	g.visibility = GITLAB_GROUP_PUBLIC;
	CPPUNIT_ASSERT(m_gitlab_client->create_group(g, res)
		|| m_gitlab_client->get_http_code() == 502);

	MARK_GITLAB_FAILURE_ON_INIT_IF
}

void Test_Gitlab::get_group()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED
	Json::Value result;
	CPPUNIT_ASSERT(m_gitlab_client->get_group(std::string("ww_testgroup_")
		+ RUN_TIMESTAMP, result) == GITLAB_RC_OK
		|| m_gitlab_client->get_http_code() == 502);
	MARK_GITLAB_FAILURE_ON_INIT_IF
}

void Test_Gitlab::remove_group()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED
	GitlabRetCod rc = m_gitlab_client->delete_group(TEST_GROUP);
	CPPUNIT_ASSERT(rc == GITLAB_RC_OK || rc == GITLAB_RC_NOT_FOUND
		|| m_gitlab_client->get_http_code() == 502);
	MARK_GITLAB_FAILURE_ON_INIT_IF
}

void Test_Gitlab::remove_groups()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED

	GitlabRetCod rc = m_gitlab_client->delete_groups(
		{"ww_testgroup_default_" + RUN_TIMESTAMP,
			"ww_testgroup2_default_" + RUN_TIMESTAMP});

	MARK_GITLAB_FAILURE_ON_INIT_IF

	ONLY_IF_GITLAB_INIT_SUCCEED

	std::string message = "Unable to remove groups. rc: " + std::to_string(rc)
		+ " http code: " + std::to_string(m_gitlab_client->get_http_code());
	CPPUNIT_ASSERT_MESSAGE(message, rc == GITLAB_RC_OK || rc == GITLAB_RC_NOT_FOUND);
}

void Test_Gitlab::get_namespaces()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED
	Json::Value result;
	CPPUNIT_ASSERT(m_gitlab_client->get_namespaces("", result) == GITLAB_RC_OK);
}

void Test_Gitlab::get_namespace()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED
	Json::Value result;
	CPPUNIT_ASSERT(
			m_gitlab_client->get_namespace(
					std::string("ww_testgroup_") + RUN_TIMESTAMP,
					result) == GITLAB_RC_OK);

	m_testing_namespace_id = result["id"].asUInt();
}

void Test_Gitlab::create_default_projects()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED
	Json::Value res;
	CPPUNIT_ASSERT(m_gitlab_client->create_project(
			GitlabProject("ww_testproj1_default_" + RUN_TIMESTAMP), res) ==
				   GITLAB_RC_OK || m_gitlab_client->get_http_code() == 502);

	MARK_GITLAB_FAILURE_ON_INIT_IF

	ONLY_IF_GITLAB_INIT_SUCCEED

	CPPUNIT_ASSERT(m_gitlab_client->create_project(
			GitlabProject("ww_testproj2_default_" + RUN_TIMESTAMP), res) ==
				   GITLAB_RC_OK || m_gitlab_client->get_http_code() == 502);

	MARK_GITLAB_FAILURE_ON_INIT_IF
}

void Test_Gitlab::create_project()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED
	// Required to get the namespace on which create the project
	get_namespace();

	Json::Value res;
	GitlabProject proj("ww_testproj_" + RUN_TIMESTAMP);
	proj.builds_enabled = false;
	proj.visibility_level = GITLAB_PROJECT_INTERNAL;
	proj.merge_requests_enabled = false;
	proj.issues_enabled = true;
	proj.lfs_enabled = true;
	proj.description = "Amazing description";
	proj.only_allow_merge_if_all_discussions_are_resolved = true;
	proj.namespace_id = m_testing_namespace_id;
	CPPUNIT_ASSERT(m_gitlab_client->create_project(proj, res) == GITLAB_RC_OK);
}

void Test_Gitlab::get_projects()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED
	Json::Value res;
	CPPUNIT_ASSERT(m_gitlab_client->get_projects("ww_testproj", res) == GITLAB_RC_OK);
	CPPUNIT_ASSERT(res[0].isMember("http_url_to_repo"));
}

void Test_Gitlab::get_project()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED
	Json::Value res;
	CPPUNIT_ASSERT(
			m_gitlab_client->get_project("ww_testproj1_default_" + RUN_TIMESTAMP,
					res) == GITLAB_RC_OK);
	CPPUNIT_ASSERT(res.isMember("name_with_namespace"));
}

void Test_Gitlab::get_project_ns()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED
	Json::Value res;
	CPPUNIT_ASSERT(m_gitlab_client->get_project_ns("ww_testproj_" + RUN_TIMESTAMP,
			TEST_GROUP, res) == GITLAB_RC_OK);
	CPPUNIT_ASSERT(res.isMember("avatar_url"));
}

void Test_Gitlab::remove_project()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED

	GitlabRetCod rc = m_gitlab_client->delete_project(std::string("ww_testproj_") +
		RUN_TIMESTAMP);
	MARK_GITLAB_FAILURE_ON_INIT_IF

	ONLY_IF_GITLAB_INIT_SUCCEED

	CPPUNIT_ASSERT(rc == GITLAB_RC_OK);
}

void Test_Gitlab::remove_projects()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED

	GitlabRetCod rc = m_gitlab_client->delete_projects(
		{"ww_testproj1_default_" + RUN_TIMESTAMP,
			"ww_testproj2_default_" + RUN_TIMESTAMP});

	MARK_GITLAB_FAILURE_ON_INIT_IF

	ONLY_IF_GITLAB_INIT_SUCCEED

	CPPUNIT_ASSERT(rc == GITLAB_RC_OK || rc == GITLAB_RC_NOT_FOUND);
}

void Test_Gitlab::create_label()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED
	Json::Value result;
	CPPUNIT_ASSERT(
			m_gitlab_client->create_label(TEST_GROUP, "ww_testproj_" + RUN_TIMESTAMP,
					TEST_LABEL, TEST_COLOR, result) ==
			GITLAB_RC_OK);
}

void Test_Gitlab::get_label()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED
	Json::Value result;
	CPPUNIT_ASSERT(m_gitlab_client->get_label(TEST_GROUP,
			"ww_testproj_" + RUN_TIMESTAMP,
			TEST_LABEL, result) == GITLAB_RC_OK);
	CPPUNIT_ASSERT(result.isMember("color") &&
				   result["color"].asString() == TEST_COLOR);
}

void Test_Gitlab::remove_label()
{
	PRECHECK_GITLAB_CLIENT;

	ONLY_IF_GITLAB_INIT_SUCCEED
	CPPUNIT_ASSERT(m_gitlab_client->delete_label(TEST_GROUP,
			"ww_testproj_" + RUN_TIMESTAMP,
			TEST_LABEL) == GITLAB_RC_OK);
}

}
}
