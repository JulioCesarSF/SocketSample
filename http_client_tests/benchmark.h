#pragma once

#include <string>
#include <chrono>

class benchmark_t
{
private:
	typedef std::chrono::high_resolution_clock clock_t;
	typedef std::chrono::duration<double, std::ratio<1>> seconds_t;

	std::chrono::time_point<clock_t> _timer;

public:
	benchmark_t() : _timer(clock_t::now()) {}

	void reset();

	double elapsed() const;
};
