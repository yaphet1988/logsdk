#include "xmutex.h"
#include "logger.h"

#if !defined(LOG_OS_WIN)
#import <pthread.h>
#endif

Mutex::Mutex() {
#if defined(LOG_OS_WIN)
	InitializeCriticalSection( &TheCS );
#else
	pthread_mutex_init( &TheMutex, NULL );      
#endif
}

Mutex::~Mutex() {
#if defined(LOG_OS_WIN)
	DeleteCriticalSection( &TheCS );
#else
	pthread_mutex_destroy( &TheMutex );
#endif
    }

void Mutex::lock() {
#if defined(LOG_OS_WIN)
	if ( !TryEnterCriticalSection( &TheCS ) ) 
		EnterCriticalSection( &TheCS );
#else
	pthread_mutex_lock( &TheMutex );
#endif
    }

void Mutex::unlock() {
#if defined(LOG_OS_WIN)
	LeaveCriticalSection( &TheCS );
#else
	pthread_mutex_unlock( &TheMutex );
#endif
    }


