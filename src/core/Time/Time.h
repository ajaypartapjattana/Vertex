#pragma once

#include <cstdint>

struct Time {
	static uint64_t now_ns();
	static double now();
};