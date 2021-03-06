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

#include "test_threads.h"

#include <core/utils/threads.h>
#include <core/utils/threadpool.h>

namespace winterwind {
namespace unittests {

class ThreadTest : public Thread
{
public:
	virtual void *run()
	{
		ThreadStarted();
		std::this_thread::sleep_for(std::chrono::seconds(5));
		return nullptr;
	}
};

struct WorkerIn
{
	uint32_t a = 0;
	uint32_t b = 0;
};

struct WorkerOut : public WorkerIn
{
	uint32_t r = 0;
};

class TPTestWorker : public Thread
{
public:
	virtual void *run()
	{
		ThreadStarted();
		CPPUNIT_ASSERT(m_work_queue);
		while (!is_stopping()) {
			while (!m_work_queue->input_queue_empty()) {
				WorkerIn wi = m_work_queue->read_input_queue();
				WorkerOut wo;
				wo.a = wi.a;
				wo.b = wi.b;
				wo.r = wi.a + wi.b;
				m_work_queue->write_output(wo);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		return nullptr;
	}

	void set_work_queue(ThreadPoolWorkQueue<TPTestWorker, WorkerIn, WorkerOut> *t)
	{
		m_work_queue = t;
	}

private:
	ThreadPoolWorkQueue<TPTestWorker, WorkerIn, WorkerOut> *m_work_queue = nullptr;
};

class TPWQTest : public ThreadPoolWorkQueue<TPTestWorker, WorkerIn, WorkerOut>
{
public:
	explicit TPWQTest(const uint32_t tn) : ThreadPoolWorkQueue(tn) {}
};

void Test_Threads::thread_pool()
{
	std::unique_ptr<ThreadPool<ThreadTest>> tp = std::make_unique<ThreadPool<ThreadTest>>(4);
	CPPUNIT_ASSERT(tp);
	tp->start_threads();
	tp->stop_threads(true);
}

void Test_Threads::working_queue()
{
	std::unique_ptr<TPWQTest> wq = std::make_unique<TPWQTest>(6);
	CPPUNIT_ASSERT(wq);

	static const uint8_t TEST_INPUT_NUMBERS = 100;
	for (uint8_t i = 0; i < TEST_INPUT_NUMBERS; i++) {
		wq->write_input({i, i});
	}

	CPPUNIT_ASSERT(wq->input_queue_size() == TEST_INPUT_NUMBERS);

	wq->start_threads();

	// 1 second wait should be sufficient for threads to process the queue
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// Stop workers (waiting them)
	wq->stop_threads();
	CPPUNIT_ASSERT(wq->output_queue_size() == TEST_INPUT_NUMBERS);
	while (!wq->output_queue_empty()) {
		WorkerOut wo = wq->read_output();
		CPPUNIT_ASSERT(wo.a + wo.b == wo.r);
	}
}
}
}
