#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

namespace sgb {

	class ThreadPool {
	public:
		using Task = std::function<void()>;

		ThreadPool(size_t threadCount = std::thread::hardware_concurrency());
		~ThreadPool();

		void enqueue(Task task);
		void stop();

	private:
		void workerLoop();

	private:
		std::vector<std::thread> m_threads;
		std::queue <Task> m_tasks;

		std::mutex m_mutex;
		std::condition_variable m_cv;

		bool m_stopping = false;

	};

}

