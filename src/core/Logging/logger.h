#pragma once

#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>

enum class logLevel : uint8_t {
	TRACE,
	INFO,
	WARN,
	ERROR,
	FATAL
};

struct LogEntry {
	char data[128];
	uint16_t size;
	logLevel level;
};

namespace sgb {

	class logger {
	public:
		static logger& instance() noexcept;

		void start(uint32_t capacity);
		void stop();

		void log(logLevel level, const char* msg, size_t size);

	private:
		logger() = default;
		~logger();

		void process();

		const char* levelToString(logLevel level) const;

	private:
		static constexpr uint32_t BUFFER_SIZE = 128;

		std::vector<LogEntry> m_buffer;
		std::thread m_thread;

		uint32_t m_capacity = 0;
		std::atomic<bool> m_running{ false };

		alignas(64) std::atomic<uint32_t> m_writeIndex{ 0 };
		alignas(64) std::atomic<uint32_t> m_readIndex{ 0 };

		std::condition_variable m_cv;
		std::mutex m_cvMutex;

	};

}
