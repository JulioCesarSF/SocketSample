#include "benchmark.h"

void benchmark_t::reset()
{
	_timer = clock_t::now();
}

double benchmark_t::elapsed() const
{
	return std::chrono::duration_cast<seconds_t>(clock_t::now() - _timer).count();
}
