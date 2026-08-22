#include "logger.h"

#include <cstring>
#include <algorithm>
#include <iostream>

namespace sgb {

	logger& logger::instance() noexcept {
		static logger inst;
		return inst;
	}

	logger::~logger() {
		stop();
	}

	void logger::start(uint32_t capacity) {
		m_capacity = capacity;
		m_buffer.resize(capacity);

		m_writeIndex.store(0, std::memory_order_relaxed);
		m_readIndex.store(0, std::memory_order_relaxed);

		m_running = true;
		m_thread = std::thread(&logger::process, this);
	}

	void logger::stop() {
		m_running = false;
		if (m_thread.joinable())
			m_thread.join();
	}

	void logger::log(logLevel level, const char* msg, size_t size) {
		uint32_t write = m_writeIndex.load(std::memory_order_relaxed);
		uint32_t read = m_readIndex.load(std::memory_order_acquire);

		if (write - read >= m_capacity) {
			return;
		}

		uint32_t idx = m_writeIndex.fetch_add(1, std::memory_order_release);

		LogEntry& entry = m_buffer[idx % m_capacity];

		entry.level = level;
		entry.size = static_cast<uint16_t>(std::min<size_t>(size, sizeof(entry.data)));

		std::memcpy(entry.data, msg, entry.size);

		m_cv.notify_one();
	}

	void logger::process() {
		while (true) {
			std::unique_lock<std::mutex> lock(m_cvMutex);

			m_cv.wait(lock, [this] {
				return !m_running.load(std::memory_order_acquire) || m_readIndex.load(std::memory_order_acquire) < m_writeIndex.load(std::memory_order_acquire);
				});

			lock.unlock();

			uint32_t read = m_readIndex.load(std::memory_order_acquire);
			uint32_t write = m_writeIndex.load(std::memory_order_acquire);

			while (read < write) {
				uint32_t idx = m_readIndex.fetch_add(1, std::memory_order_acq_rel);

				LogEntry& entry = m_buffer[idx % m_capacity];

				std::ostream& out = (entry.level >= logLevel::ERROR) ? std::cerr : std::cout;

				out << "[" << levelToString(entry.level) << "] ";
				out.write(entry.data, entry.size);
				out << '\n';

				read++;
			}

			if (!m_running.load(std::memory_order_acquire) && m_readIndex.load(std::memory_order_acquire) >= m_writeIndex.load(std::memory_order_acquire))
				break;
		}
	}

	const char* logger::levelToString(logLevel level) const {
		switch (level) {
		case logLevel::TRACE:	return "TRACE";
		case logLevel::INFO:	return "INFO";
		case logLevel::WARN:	return "WARN";
		case logLevel::ERROR:	return "ERROR";
		case logLevel::FATAL:	return "FATAL";
		default: return "UNKNOWN";
		}
	}

}
