#include "threadPool.h"

namespace sgb {

	ThreadPool::ThreadPool(size_t threadCount) {
		for (size_t i = 0; i < threadCount; ++i) {
			m_threads.emplace_back([this] { workerLoop(); });
		}
	}

	ThreadPool::~ThreadPool() {
		stop();
	}

	void ThreadPool::enqueue(Task task) {
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_tasks.push(std::move(task));
		}

		m_cv.notify_one();
	}

	void ThreadPool::stop() {
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_stopping = true;
		}

		m_cv.notify_all();

		for (auto& t : m_threads) {
			if (t.joinable())
				t.join();
		}
	}

	void ThreadPool::workerLoop() {
		while (true) {
			Task task;
			{
				std::unique_lock<std::mutex> lock(m_mutex);

				m_cv.wait(lock, [this] {
					return m_stopping || !m_tasks.empty();
					});

				if (m_stopping && m_tasks.empty())
					return;

				task = std::move(m_tasks.front());
				m_tasks.pop();
			}

			task();
		}
	}

}

