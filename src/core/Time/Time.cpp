#include "Time.h"

#include <chrono>

uint64_t Time::now_ns() {
	static const auto start = std::chrono::steady_clock::now();
	
	auto now = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
}

double Time::now() {
	return now_ns() * 1e-9;
}