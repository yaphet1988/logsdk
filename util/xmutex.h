#pragma once
#include "logger.h"

#if defined(LOG_OS_WIN)
#include <Windows.h>
#else
#include <mutex>
#endif

class Mutex
{
public:
    Mutex();
    ~Mutex();

public:
    void lock();
    void unlock();
    
private:
#if defined(LOG_OS_WIN)
    mutable CRITICAL_SECTION TheCS;
#else
    mutable pthread_mutex_t TheMutex;
#endif
};

