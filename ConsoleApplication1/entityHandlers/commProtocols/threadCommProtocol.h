#pragma once

#include <mutex>
#include <queue>
#include <optional>

template<typename T>

class ThreadSafeQueue{
public:
	void push(T value) {
		std::lock_guard<std::mutex> lock(mutex);
		queue.push(std::move(value));
	}
	std::optional<T> try_pop() {
		std::lock_guard<std::mutex> lock(mutex);
		if (queue.empty()) return std::nullopt;
		T value = std::move(queue.front());
		queue.pop();
		return value;
	}

private:
	std::mutex mutex;
	std::queue<T> queue;
};
