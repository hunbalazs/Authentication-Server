#include "ThreadsUtils.h"

void Thread::Wait(uint32_t millisecs)
{
    #ifdef _WIN32
        Sleep(millisecs);
    #else
        usleep(millisecs * 1000);
    #endif
}

uint64_t Thread::GetTicks()
{
    #ifdef _WIN32
    return GetTickCount();
    #else
     struct timeval tv;
     if (gettimeofday(&tv, NULL) != 0)
     {
         return 0;
     }
     return (tv.tv_sec *1000) + (tv.tv_usec / 1000);
     #endif
}
