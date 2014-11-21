#ifndef TR_THREADS_UTILS_H
#define TR_THREADS_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#ifndef _WIN32
#include <sys/time.h>
#include <unistd.h>
#endif

namespace Thread
{
	void Wait(uint32_t millisecs);
	uint64_t GetTicks();
}
#endif
