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

#include "utils/threads.h"
#include <iostream>
#include <memory>

class ConsoleHandler;

enum CommandChannel
{
	COMMAND_CHANNEL_NONE,
	COMMAND_CHANNEL_UNITTESTS,
	COMMAND_CHANNEL_STDIN,
	COMMAND_CHANNEL_IRC,
};

struct CommandToProcess
{
public:
	CommandToProcess(CommandChannel ch, const char* cmd):
		channel(ch), command(std::string(cmd)) {}

	CommandToProcess(CommandChannel ch, const char* ch_extra_info,
			const char* w, const char* cmd):
		channel(ch),
		channel_extra_info(std::string(ch_extra_info)),
		who(std::string(w)),
		command(std::string(cmd)) {}

	CommandToProcess(const CommandToProcess &c):
		channel(c.channel),
		channel_extra_info(c.channel_extra_info),
		who(c.who),
		command(c.command) {}

	CommandChannel channel = COMMAND_CHANNEL_NONE;
	std::string channel_extra_info = "";
	std::string who = "";
	std::string command = "";
};

typedef std::shared_ptr<CommandToProcess> CommandToProcessPtr;

enum RemoteCommandType
{
	REMOTECOMMAND_NONE,
	REMOTECOMMAND_ONLY,
	REMOTECOMMAND_BOTH,
};

enum CommandArgsMode
{
	COMMANDARGS_NONE,
	COMMANDARGS_BOTH,
	COMMANDARGS_DYNAMIC,
	COMMANDARGS_STATIC,
};

struct ChatCommandHandlerArg
{
public:
	ChatCommandHandlerArg(const std::string &w, const std::string &c, const std::string &a):
		who(w), channel(c), args(a) {}
	std::string who = "";
	std::string channel = "";
	std::string args = "";
};

template <class C>
struct ChatCommand
{
	const char* name;
	bool (C::*handler)(const ChatCommandHandlerArg& c_arg, std::stringstream &ss);
	ChatCommand* childCommand;
	RemoteCommandType rct;
	CommandArgsMode cam;
	const std::string help;
	const bool show_help;
};

enum ChatCommandSearchResult
{
	CHAT_COMMAND_OK,                    // found accessible command by command string
	CHAT_COMMAND_UNKNOWN,               // first level command not found
	CHAT_COMMAND_UNKNOWN_SUBCOMMAND,    // command found but some level subcommand not find in subcommand list
};

class ConsoleHandler
{
public:
	ConsoleHandler() {}
	virtual ~ConsoleHandler() {}
	virtual void enqueue(CommandToProcessPtr cmd) = 0;
	virtual void process_queue() = 0;
protected:
	virtual bool handle_command(const CommandToProcessPtr cmd, std::stringstream &ss) = 0;
	virtual bool handle_command_help(const ChatCommandHandlerArg &args, std::stringstream &ss) = 0;
};

class ConsoleThread: public Thread
{
public:
	ConsoleThread(ConsoleHandler *hdl, const std::string &&prompt):
		Thread(), m_console_handler(hdl), m_prompt(prompt) {}

	~ConsoleThread() {}

	void * run();
private:
	ConsoleHandler* m_console_handler = nullptr;
	std::string m_prompt = "";
};
