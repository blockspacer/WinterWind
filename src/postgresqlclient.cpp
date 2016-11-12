/**
 * Copyright (c) 2016, Loic Blot <loic.blot@unix-experience.fr>
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

#include <sstream>
#include <iostream>
#include <array>
#include "postgresqlclient.h"

PostgreSQLClient::PostgreSQLClient(const std::string &connect_string,
	int32_t minimum_db_version):
	m_connect_string(connect_string),
	m_min_pgversion(minimum_db_version)
{
	connect();
}

PostgreSQLClient::~PostgreSQLClient()
{
	if (m_conn) {
		PQfinish(m_conn);
	}
}

void PostgreSQLClient::connect()
{
	m_conn = PQconnectdb(m_connect_string.c_str());

	if (PQstatus(m_conn) != CONNECTION_OK) {
		throw PostgreSQLException(std::string(
			"PostgreSQL database error: ") + PQerrorMessage(m_conn));
	}

	m_pgversion = PQserverVersion(m_conn);
	if (m_pgversion < m_min_pgversion) {
		std::stringstream ss;
		ss << "PostgreSQL database error: Server version " << m_pgversion
		   << " < required version " << m_min_pgversion;
		throw PostgreSQLException(ss.str());
	}

	check_db_connection();
}

void PostgreSQLClient::check_db_connection()
{
	if (PQstatus(m_conn) == CONNECTION_OK)
		return;

	if (PQping(m_connect_string.c_str()) != PQPING_OK) {
		throw PostgreSQLException(std::string("PostgreSQL database error: ") +
			PQerrorMessage(m_conn));
	}

	PQresetStart(m_conn);
}

void PostgreSQLClient::begin()
{
	check_db_connection();
	PQexec(m_conn, "BEGIN;");
}

void PostgreSQLClient::commit()
{
	PQexec(m_conn, "COMMIT;");
}

void PostgreSQLClient::set_client_encoding(const std::string &encoding)
{
	static const std::array<std::string, 2> allowed_encoding = {"LATIN1", "UTF8"};
	bool valid = false;
	for (const auto &allowed_value: allowed_encoding) {
		if (allowed_value.compare(encoding) == 0) {
			valid = true;
			break;
		}
	}
	if (!valid) {
		throw PostgreSQLException(std::string(
			"PostgreSQLClient: Invalid client_encoding provided"));
	}

	std::string request = "SET client_encoding='";
	request += encoding;
	request += "';";
	check_results(PQexec(m_conn, request.c_str()));
}

PGresult *PostgreSQLClient::check_results(PGresult *result, bool clear)
{
	ExecStatusType statusType = PQresultStatus(result);

	switch (statusType) {
		case PGRES_COMMAND_OK:
		case PGRES_TUPLES_OK:
			break;
		case PGRES_FATAL_ERROR:
		default:
			throw PostgreSQLException(std::string("PostgreSQL database error: ") +
				PQresultErrorMessage(result));
	}

	if (clear) {
		PQclear(result);
	}

	return result;
}

bool PostgreSQLClient::register_statement(const std::string &stn, const std::string &st)
{
	if (m_statements.find(stn) != m_statements.end()) {
		std::cerr << "WARN: Trying to register statement " << stn
				<< ", but it's already registered, ignoring." << std::endl;
		return false;
	}

	check_results(PQprepare(m_conn, stn.c_str(), st.c_str(), 0, NULL));

	m_statements[stn] = st;
	return true;
}