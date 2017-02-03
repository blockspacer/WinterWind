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

#pragma once

#include <string>
#include <libpq-fe.h>
#include <unordered_map>
#include <stdlib.h>
#include <vector>
#include "utils/exception.h"

class PostgreSQLException: public BaseException
{
public:
	PostgreSQLException(const std::string &what): BaseException(what) {}
	~PostgreSQLException() throw() {}
};

class PostgreSQLClient;
/**
 * RAII class for PostgreSQL results
 */
class PostgreSQLResult
{
public:
	PostgreSQLResult(PostgreSQLClient *client, PGresult *result);
	~PostgreSQLResult() { PQclear(m_result); }

	PGresult * operator*() { return m_result; }
	ExecStatusType get_status() const { return m_status; }
private:
	PGresult *m_result = nullptr;
	ExecStatusType m_status = PGRES_COMMAND_OK;
};

struct PostgreSQLTableField
{
	std::string column_name = "";
	std::string data_type = "";
	bool is_nullable = false;
	std::string column_default = "";
};

struct PostgreSQLTableDefinition
{
	std::vector<PostgreSQLTableField> fields;
	std::unordered_map<std::string, std::string> indexes;
};

class PostgreSQLClient
{
	friend class PostgreSQLResult;
public:
	PostgreSQLClient(const std::string &connect_string,
		int32_t minimum_db_version = 90500);
	virtual ~PostgreSQLClient();

	void begin();
	void commit();

	bool register_statement(const std::string &stn, const std::string &st);
	void register_embedded_statements();

	ExecStatusType show_schemas(std::vector<std::string> &res);
	ExecStatusType create_schema(const std::string &name);
	ExecStatusType drop_schema(const std::string &name, bool if_exists=false);
	ExecStatusType show_tables(const std::string &schema, std::vector<std::string> &res);
	ExecStatusType show_create_table(const std::string &schema, const std::string &table,
		PostgreSQLTableDefinition &definition);

	ExecStatusType add_admin_views(const std::string &schema = "admin");
	void escape_string(const std::string &param, std::string &res);

	const std::string &get_last_error() const { return m_last_error; }
protected:
	void check_db_connection();
	void set_client_encoding(const std::string &encoding);
	PGresult *check_results(PGresult *result, bool clear = true);

	inline int pg_to_int(PGresult *res, int row, int col)
	{
		return atoi(PQgetvalue(res, row, col));
	}

	inline const std::string pg_to_string(PGresult *res, int row, int col) {
		return std::string(PQgetvalue(res, row, col), PQgetlength(res, row, col));
	}

	inline const uint32_t pg_to_uint(PGresult *res, int row, int col) {
		return (const uint32_t) atoi(PQgetvalue(res, row, col));
	}

	inline const uint64_t pg_to_uint64(PGresult *res, int row, int col) {
		return (const uint64_t) atoll(PQgetvalue(res, row, col));
	}

	inline const int64_t pg_to_int64(PGresult *res, int row, int col) {
		unsigned char* data = (unsigned char*)PQgetvalue(res, row, col);
		return (const int64_t) atoll((char*) data);
	}

	PGresult *exec_prepared(const char *stmtName, const int paramsNumber,
		const char **params, const int *paramsLengths = NULL,
		const int *paramsFormats = NULL,
		bool clear = true, bool nobinary = true)
	{
		return check_results(PQexecPrepared(m_conn, stmtName, paramsNumber,
			(const char* const*) params, paramsLengths, paramsFormats,
			nobinary ? 1 : 0), clear);
	}

private:
	void connect();

	std::string m_connect_string = "";
	PGconn *m_conn = nullptr;
	int32_t m_pgversion = 0;
	int32_t m_min_pgversion = 0;

	std::string m_last_error = "";

	std::unordered_map<std::string, std::string> m_statements;
};
