
#include "stdafx.h"

#include "thread.h"

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
