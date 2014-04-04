
#include "stdafx.h"

#include "particle.h"

//------------- Thread --------------//

struct _ParticleThread {
#ifdef _WINDOWS
	HANDLE thread;
#else
	pthread_t thread;
#endif
};

typedef struct _ThreadFunction {
	size_t (*fn)(void *);
	void *data;
} ThreadFunction;

#ifdef _WINDOWS
DWORD WINAPI run_wrapper(LPVOID param) {
#else
void *run_wrapper(void *param) {
#endif
	ThreadFunction *tf = (ThreadFunction *) param;
	size_t (*fn)(void *) = tf->fn;
	void *data = tf->data;

	free(tf);
	fn(data);

	return 0;
}

ParticleThread *particle_thread_create(size_t (*fn)(void *), void *data)
{
	ThreadFunction *tf = (ThreadFunction *) malloc(sizeof(ThreadFunction));
	ParticleThread *pt = (ParticleThread *) malloc(sizeof(ParticleThread));

	tf->fn = fn;
	tf->data = data;

#ifdef _WINDOWS
	pt->thread = CreateThread(0,0,run_wrapper,tf,0,0);
#else
	pthread_create(&pt->thread, NULL,&run_wrapper,tf);
#endif

	return pt;
}

size_t particle_thread_join(ParticleThread *pt)
{
	size_t retval = 0;
#ifdef _WINDOWS
	unsigned long windowsisadick;
	WaitForSingleObject(pt->thread, INFINITE);
	GetExitCodeThread(pt->thread,&windowsisadick);
	CloseHandle(pt->thread);
	retval = windowsisadick;
#else
	pthread_join(pt->thread, (void **) &retval);
#endif
	free(pt);

	return retval;
}

//------------- Mutex ---------------//


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

//------- Condition Variable --------//

struct _PCV {
	ParticleMutex *m;

#ifdef _WINDOWS
	CONDITION_VARIABLE var;
#else
	pthread_cond_t var;
#endif
};

PCV *particle_condition_var_new(ParticleMutex *m)
{
	PCV *v = (PCV *) malloc(sizeof(PCV));
	v->m = m;

#ifdef _WINDOWS
	InitializeConditionVariable(&v->var);
#else
	pthread_cond_init(&v->var,0);
#endif

	return v;
}

void particle_condition_var_free(PCV *v)
{
	if(v) {
#ifndef _WINDOWS
		pthread_cond_destroy(&v->var);
#endif

		free(v);
	}
}

void particle_condition_var_sleep(PCV *v)
{
#ifdef _WINDOWS
	SleepConditionVariableCS(&v->var, &v->m->mutex, INFINITE);
#else
	pthread_cond_wait(&v->var,&v->m->mutex);
#endif
}

void particle_condition_var_wake(PCV *v)
{
#ifdef _WINDOWS
	WakeAllConditionVariable(&v->var);
#else
	pthread_cond_broadcast(&v->var);
#endif
}

void particle_condition_var_wake_single(PCV *v)
{
#ifdef _WINDOWS
	WakeConditionVariable(&v->var);
#else
	pthread_cond_signal(&v->var);
#endif
}
