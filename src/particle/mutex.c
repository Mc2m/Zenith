
#include "stdafx.h"

#include "mutex.h"

struct _ParticleMutex {
#ifdef _WINDOWS
	CRITICAL_SECTION mutex;
#else
	pthread_mutexattr_t mutexAttr;
	pthread_mutex_t mutex;
#endif
	unsigned int type;
};

ParticleMutex *particle_mutex_new(unsigned int type)
{
	ParticleMutex *pm = (ParticleMutex *) malloc(sizeof(ParticleMutex));
	pm->type = type;

#ifdef _WINDOWS
	InitializeCriticalSection(&pm->mutex);
#else
	int error = pthread_mutexattr_init(&pm->mutexAttr);
	assert(error == 0);

	if(type == PARTICLE_MUTEX_TYPE_NORMAL) {
		type = PTHREAD_MUTEX_NORMAL;
	} else if (type == PARTICLE_MUTEX_TYPE_RECURSIVE) {
		type = PTHREAD_MUTEX_RECURSIVE;
	} else if (type == PARTICLE_MUTEX_TYPE_ERRORCHECK) {
		type = PTHREAD_MUTEX_ERRORCHECK;
	} else if (type == PARTICLE_MUTEX_TYPE_READWRITE) {
		type = PTHREAD_PROCESS_PRIVATE;
	}

	error = pthread_mutexattr_settype(&pm->mutexAttr, type);
	assert(error == 0);
	error = pthread_mutex_init(&pm->mutex, &pm->mutexAttr);
	assert(error == 0);
#endif

	return pm;
}

void particle_mutex_free(ParticleMutex *m)
{
#ifdef _WINDOWS
	DeleteCriticalSection(&m->mutex);
#else
	pthread_mutex_destroy(&m->mutex);
	pthread_mutexattr_destroy(&m->mutexAttr);
#endif

	free(m);
}

void particle_mutex_lock(ParticleMutex *m)
{
#ifdef _WINDOWS
	EnterCriticalSection(&m->mutex);
#else
	assert(pthread_mutex_lock(&m->mutex) == 0);
#endif
}

void particle_mutex_unlock(ParticleMutex *m)
{
#ifdef _WINDOWS
	LeaveCriticalSection(&m->mutex);
#else
	assert(pthread_mutex_unlock(&m->mutex) == 0);
#endif
}
